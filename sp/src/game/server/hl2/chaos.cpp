#include "chaos.h"

#include "cbase.h"
#include "hl2_player.h"
#include "movevars_shared.h"
#include "dt_utlvector_send.h"

// memdbgon must be the last include file in a .cpp file!!! according to the government
#include "tier0/memdbgon.h"

//BEGIN_SIMPLE_DATADESC(CChaosEffect)
//DEFINE_FIELD(m_nID, FIELD_INTEGER),
//DEFINE_FIELD(m_strHudName, FIELD_STRING),
//DEFINE_FIELD(m_nGroup, FIELD_INTEGER),
//DEFINE_FIELD(m_nContext, FIELD_INTEGER),
//DEFINE_FIELD(m_flTimeRem, FIELD_FLOAT),
//END_DATADESC()

BEGIN_SEND_TABLE_NOBASE( CChaosEffect, DT_ChaosEffect )
	SendPropString( SENDINFO( m_strName ) ),
	SendPropFloat( SENDINFO( m_flTimeRem ) ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CChaosController, DT_ChaosController )
	SendPropFloat( SENDINFO( m_flNextEffectRem ) ),
	SendPropUtlVector(
		SENDINFO_UTLVECTOR( m_activeEffects ),
		MAX_ACTIVE_EFFECTS,
		SendPropDataTable( NULL, 0, &REFERENCE_SEND_TABLE( DT_ChaosEffect ), SendProxy_DataTablePtrToDataTable )
	)
END_SEND_TABLE()


LINK_ENTITY_TO_CLASS( chaos_controller, CChaosControllerProxy );
IMPLEMENT_SERVERCLASS( CChaosControllerProxy, DT_ChaosControllerProxy )


void* SendProxy_ChaosController( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetAllRecipients();
	return &g_chaosController;
}
CChaosControllerProxy* CChaosControllerProxy::s_pChaosControllerProxy = nullptr;

BEGIN_SEND_TABLE( CChaosControllerProxy, DT_ChaosControllerProxy )
	SendPropDataTable( "chaos_controller_data", 0, &REFERENCE_SEND_TABLE( DT_ChaosController ), SendProxy_ChaosController )
END_SEND_TABLE()


void CChaosControllerProxy::NotifyNetworkStateChanged()
{
	if ( s_pChaosControllerProxy )
		s_pChaosControllerProxy->NetworkStateChanged();
}
int CChaosControllerProxy::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

ConVar chaos("chaos", "0", FCVAR_NONE);
ConVar chaos_effect_time("chaos_effect_time", "90", FCVAR_NONE, "Standard effect length.");
ConVar chaos_instant_off("chaos_instant_off", "0", FCVAR_NONE);
ConVar chaos_print_rng("chaos_print_rng", "0");
ConVar chaos_vote_enable("chaos_vote_enable", "0");
ConVar chaos_effect_interval("chaos_effect_interval", "30", FCVAR_REPLICATED, "Time between each effect.");
ConVar chaos_unstuck_neweffect("chaos_unstuck_neweffect", "1", FCVAR_NONE, "Get the player unstuck every time a new effect starts. may not be wanted by some technical players.");

CChaosController g_chaosController;

// TODO: all of those should go away.
int							g_iChaosSpawnCount = 0;
CUtlVector<int>				g_iTerminated;//list of chaos ids to NOT restore from txt. used to remember which NPCs are dead as it would not make sense for them to come back to life.
CUtlVector<CChaosStoredEnt *> g_PersistEnts;

bool						g_bGoBackLevel = false;
bool						g_bGroupsMade = false;

#ifdef DEBUG
CON_COMMAND(cte, "turn on a specific effect")
#else
CON_COMMAND(chaos_test_effect, "turn on a specific effect")
#endif // DEBUG
{
	if (args.ArgC() <= 1) {
		return;
	}
	int effectID = atoi(args[1]);
	if (effectID < NUM_EFFECTS)
	{
		g_chaosController.StartGivenEffect(effectID);
		g_chaosController.PunishEffect(effectID);
	}
}

CON_COMMAND(chaos_print, "print all effects for debugging")
{
	for (int i = 1; i < NUM_EFFECTS; i++)
	{
		Msg("%i: %s %s\n", i, g_chaosController.m_effects[i]->GetName(), 
			g_chaosController.IsEffectActive(i) ? "ACTIVE" : "");
	}
}

CON_COMMAND(chaos_test_rng, "Simulate N runs of effect choosing")
{
	if (args.ArgC() < 2)
	{
		Msg("Specify number of times to run RNG\n");
		return;
	}
	int iWeightSum = g_chaosController.GetWeightSum();
	int iPicks[NUM_EFFECTS] = { 0 }; // initialize the whole array to 0
	for (int j = 0; j < atoi(args[1]); j++)
	{
		int nID = g_chaosController.PickEffect(iWeightSum);
		iPicks[nID]++;
	}
	for (int k = 0; k < NUM_EFFECTS; k++)
	{
		Msg("%i: %s picked %i times\n", k, g_chaosController.m_effects[k]->GetName(), iPicks[k]);
	}
}


void VoteOptionsChanged(IConVar *var, const char *pOldValue, float flOldValue) 
{
	g_chaosController.ResetVotes(); // reload the options immediately
}

ConVar chaos_vote_options("chaos_vote_options", "4", FCVAR_NONE, "Sets the ammount of vote options", VoteOptionsChanged);


CON_COMMAND(chaos_vote_internal_poll, "used by an external client. returns vote number and possible choises for this vote")
{
	ConMsg("%d", g_chaosController.m_iVoteNumber);
	for (int i = 0; i < g_chaosController.m_VoteOptions.Count(); ++i)
	{
		int effectID = g_chaosController.m_VoteOptions[i].first;
		ConMsg(";%s", g_chaosController.m_effects[effectID]->GetName());
	}
}
CON_COMMAND(chaos_vote_internal_set, "used by an external client. sets current votes")
{
	if (args.ArgC() < g_chaosController.m_VoteOptions.Count() + 2) // command itself + vote number
		return;

	// client probably has outdated info. ignore him
	if (atoi(args[1]) != g_chaosController.m_iVoteNumber)
		return;

	for (int i = 0; i < g_chaosController.m_VoteOptions.Count(); ++i)
	{
		g_chaosController.m_VoteOptions[i].second = atoi(args[i+2]);
	}
}
CON_COMMAND(chaos_vote_debug, "prints info about the votes")
{
	Msg("vote#: %d\n", g_chaosController.m_iVoteNumber);
	for (int i = 0; i < g_chaosController.m_VoteOptions.Count(); ++i)
	{
		int effectID = g_chaosController.m_VoteOptions[i].first;
		Msg("%i: %s %d\n", 
			i, 
			g_chaosController.m_effects[effectID]->GetName(), 
			g_chaosController.m_VoteOptions[i].second);
	}
}

CON_COMMAND(chaos_vote_reset, "choses new effects and resets votes")
{
	g_chaosController.ResetVotes();
}

// TODO: those two gotta go.
void ClearPersistEnts()
{
	g_PersistEnts.RemoveAll();
}
void ClearChaosData()
{
	ClearPersistEnts();
	//Invert gravity messes this up
	sv_gravity.SetValue(600);
	g_iChaosSpawnCount = 0;
	
	
	g_iTerminated.RemoveAll();
	g_PersistEnts.RemoveAll();
	g_chaosController.Reset();
}

CON_COMMAND(chaos_reset, "resets stuff like sv_gravity. executes chaos_restart.cfg.")
{
	//Reset anything that persists forever
	ClearChaosData();
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "exec chaos_restart\n");
	sv_gravity.SetValue(600);
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
CON_COMMAND(chaos_restart, "restarts map and resets stuff like sv_gravity. executes chaos_restart.cfg.")
{
	//Reset anything that persists forever
	ClearChaosData();
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "restart;exec chaos_restart\n");
	g_chaosController.m_activeEffects.RemoveAll();
}

CChaosEffect::CChaosEffect(int EffectID, const char* Name, ConVar* WeightCVar, ConVar* TimeScaleCVar) 
: m_pMaxWeightCVar(WeightCVar), m_pTimeScaleCVar(TimeScaleCVar)
{
	g_chaosController.m_effects[EffectID] = this;
	m_iCurrentWeight = m_pMaxWeightCVar->GetInt();

	SetName(Name);
}

float CChaosEffect::GetSecondsRemaining()
{
	float effect_time_scale = m_pTimeScaleCVar == nullptr ? 1.0f : m_pTimeScaleCVar->GetFloat();
	return chaos_effect_time.GetFloat() * effect_time_scale * m_flTimeRem;
}

void CChaosEffect::SetNextThink(float delay)
{
	m_flThinkDelay = delay;
}

const char* CChaosEffect::GetName() {
	return m_strName.Get();
}

void CChaosEffect::SetName(const char* Name) {
	Q_strncpy( m_strName.GetForModify(), Name, MAX_EFFECT_NAME );
}

bool CChaosEffect::IterUsableVehicles(bool bFindOnly, bool bFindBoat, bool bFindBuggy)
{
	bool bFoundSomething = false;
	//there may be more than one useable car in a map cause chaos is chaotic
	//iterate on boats then cars
	CPropVehicleDriveable *pVehicle = NULL;
	if (bFindBoat)
		pVehicle = (CPropVehicleDriveable *)gEntList.FindEntityByClassname(NULL, "prop_vehicle_airboat");
	if (!pVehicle && bFindBuggy)
	{
		pVehicle = (CPropVehicleDriveable *)gEntList.FindEntityByClassname(NULL, "prop_vehicle_jeep");
		bFindBoat = false;
	}
	while (pVehicle)
	{
		//if iterating on cars, check model because some APCs use prop_vehicle_jeep
		if (!bFindBoat && bFindBuggy)
		{
			CPropJeep *pJeep = static_cast<CPropJeep *>(pVehicle);
			if (pJeep->m_bJeep || pJeep->m_bJalopy)
			{
				if (!bFindOnly)
					DoOnVehicles(pVehicle);
				bFoundSomething = true;
				if (bFindOnly)
					return true;
			}
		}
		else
		{
			if (!bFindOnly)
				DoOnVehicles(pVehicle);
			bFoundSomething = true;
			if (bFindOnly)
				return true;
		}
		//iterate on boats then cars
		if (bFindBoat)
		{
			pVehicle = (CPropVehicleDriveable *)gEntList.FindEntityByClassname(pVehicle, "prop_vehicle_airboat");
			if (!pVehicle)
				bFindBoat = false;
		}
		//in the case that we just finished iterating on the last boat, pVehicle is NULL so it should be fine to pass into FindEntityByClassname
		if (!bFindBoat)
			pVehicle = (CPropVehicleDriveable *)gEntList.FindEntityByClassname(pVehicle, "prop_vehicle_jeep");
	}
	return bFoundSomething;
}

// TODO: reimplement me!
bool EffectOrGroupAlreadyActive(int iEffect)
{
	// if (chaos_ignore_activeness.GetBool())
	// 	return false;

	// //Msg("Checking for effect number %i\n", g_ChaosEffects[iEffect]->m_nID);
	// if (g_ChaosEffects[iEffect]->m_bActive)
	// {
	// 	//Msg("Effect is already active %i\n", g_ChaosEffects[iEffect]->m_nID);
	// 	return true;
	// }

	// //not already active, but what about group
	// if (chaos_ignore_group.GetBool())
	// 	return false;

	// //check groups
	// if (groupcheck_debug.GetBool()) Msg("Checking groups for effect number %i\n", iEffect);
	// bool bNotInAnyGroup = true;
	// if (g_ChaosEffects[iEffect]->m_iExcludeCount > 0)
	// {
	// 	bNotInAnyGroup = false;
	// 	for (int i = 0; i < g_ChaosEffects[iEffect]->m_iExcludeCount; i++)
	// 	{
	// 		int iOtherEffect = g_ChaosEffects[iEffect]->m_iExclude[i];
	// 		if (g_ChaosEffects[iOtherEffect]->m_bActive)
	// 		{
	// 			if (groupcheck_debug.GetBool()) Msg("Effect %i is active, so %i cannot be chosen\n", iOtherEffect, iEffect);
	// 			return true;
	// 		}
	// 		else
	// 		{
	// 			if (groupcheck_debug.GetBool()) Msg("Effect %i is not active\n", iOtherEffect);
	// 		}
	// 	}
	// }

	// if (groupcheck_debug.GetBool() && bNotInAnyGroup) Msg("Effect %i wasn't in any group\n", iEffect);
	return false;//none in group active
}

bool CChaosController::Init()
{
	for (int i = 0; i < NUM_EFFECTS; i++)
	{
		// verify that every effect number has a class associated with it
		assert(m_effects[i]);
	}
	ResetVotes();
	m_flNextEffectRem = -1;
	return true;
}

void CChaosController::Reset()
{
	for (int i = 0; i < NUM_EFFECTS; ++i)
	{
		m_effects[i]->StopEffect();
	}
	m_activeEffects.RemoveAll();
	m_flNextEffectRem = -1;
}

int CChaosController::GetWeightSum()
{
	int iWeightSum = 0;
	for (int i = 1; i < NUM_EFFECTS; i++)
	{
		CChaosEffect* e = m_effects[i];
		if (chaos_print_rng.GetBool()) 
			Msg("i %i, %s %i += %i\n", i, e->GetName(), iWeightSum, e->m_iCurrentWeight);
		iWeightSum += e->m_iCurrentWeight;
	}
	return iWeightSum;
}

void CChaosController::RecoverWeights(float magnitute)
{
	for (int i = 1; i < NUM_EFFECTS; i++)
	{
		// recover weight for recent effects
		// add a fraction of the maximum weight on every interval
		CChaosEffect* e = m_effects[i];
		int maxWeight = e->m_pMaxWeightCVar->GetInt();
		if (!EffectOrGroupAlreadyActive(i) && e->m_iCurrentWeight < maxWeight)
		{
			int newWeight = min(
				e->m_iCurrentWeight + maxWeight * magnitute, 
				maxWeight
				);
			e->m_iCurrentWeight = newWeight;
		}
	}
}

void CChaosController::PunishEffect(int EffectID) {
	m_effects[EffectID]->m_iCurrentWeight = 0;
}

//Set the chaos_ignore_ convars if wanted
int CChaosController::PickEffect(int iWeightSum)
{
	int nRandom = 0;
	// possible to be stuck in an infinite loop, but only if there's a very small number of effects
	while (true)
	{
		//pick effect
		nRandom = random->RandomInt(0, iWeightSum);
		int nRememberRandom = nRandom;
		if (chaos_print_rng.GetBool()) Warning("nRandom is %i (%i - %i)\n", nRandom, 0, iWeightSum);
		//weights
		//start at 1 so ERROR doesn't get picked
		for (int i = 1; i < NUM_EFFECTS; i++)
		{
			if (chaos_print_rng.GetBool()) Msg("i %i, %s %i <= %i\n", i, m_effects[i]->GetName(), nRandom, m_effects[i]->m_iCurrentWeight);
			if (nRandom <= m_effects[i]->m_iCurrentWeight)
			{
				CChaosEffect *candEffect = m_effects[i];
				bool bGoodActiveness = !EffectOrGroupAlreadyActive(i);
				//check activeness and context
				if (bGoodActiveness && candEffect->CanBeChosen())
				{
					Assert(i != EFFECT_ERROR);
					if (chaos_print_rng.GetBool()) Msg("Chose effect i %i %s starting number %i\n", i, m_effects[i]->GetName(), nRememberRandom);
					return i;
				}
				else
				{
					if (chaos_print_rng.GetBool()) Msg("Breaking for loop i %i %s starting number %i\n", i, m_effects[i]->GetName(), nRememberRandom);
					break;//break here or else we just go to the next available effect down
				}
			}
			nRandom -= m_effects[i]->m_iCurrentWeight;
		}
	}
}

bool CChaosController::IsEffectActive(int EffectID)
{
	return m_activeEffects.HasElement(m_effects[EffectID]);
}

void CChaosController::StartGivenEffect(int EffectID)
{
	Assert(EffectID != EFFECT_ERROR);
	m_flNextEffectRem = chaos_effect_interval.GetFloat();
	CChaosEffect* e = m_effects[EffectID];
	Msg("Effect %s\n", e->GetName());
	e->m_flTimeRem = 1.0f;
	e->m_iStrikes = 0;
	e->m_flThinkDelay = 0.0f; // let the effect immediately think because why not
	e->StartEffect();
	m_activeEffects.AddToTail(e);
}

void CChaosController::StopGivenEffect(int EffectID)
{
	//currently need to NOT do this check so that we can abort now-inactive effects on reload
	StopGivenEffect(m_effects[EffectID]);
}

void CChaosController::StopGivenEffect(CChaosEffect* Effect)
{
	//currently need to NOT do this check so that we can abort now-inactive effects on reload
	Effect->StopEffect();

	m_activeEffects.FindAndRemove(Effect);
}

void CChaosController::ResetVotes() 
{
	m_iVoteNumber++;
	m_VoteOptions.RemoveAll();

	// save the weights so we can change them during picking to avoid duplicate picks
	int oldWeights[NUM_EFFECTS];
	for (int i = 0; i < NUM_EFFECTS; ++i)
	{
		oldWeights[i] = m_effects[i]->m_iCurrentWeight;
	}

	// choose effects to nominate
	for (int i = 0; i < chaos_vote_options.GetInt(); i++)
	{
		int iWeightSum = GetWeightSum();
		int iEffectID = PickEffect(iWeightSum);
		m_VoteOptions.AddToTail(MakeUtlPair(iEffectID, 0));
		PunishEffect(iEffectID);
	}

	// restore weights
	for (int i = 0; i < NUM_EFFECTS; ++i)
	{
		m_effects[i]->m_iCurrentWeight = oldWeights[i];
	}
}

int CChaosController::GetVoteWinnerEffect() 
{
	int bestVotes = -1;
	int bestEffect = 0;
	for (int i = 0; i < m_VoteOptions.Count(); i++)
	{
		if (m_VoteOptions[i].second > bestVotes) {
			bestVotes = m_VoteOptions[i].second;
			bestEffect = m_VoteOptions[i].first;
		}
	}
	return bestEffect;
}

void CChaosController::FrameUpdatePreEntityThink()
{
	if (gpGlobals->frametime == 0) // don't do anything if game is paused
	{
		return;
	}
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer) {
		return; // too early.
	}
	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	assert(pHL2Player);
	if (chaos.GetBool())
	{
		if (m_activeEffects.Count() == 0 && m_flNextEffectRem <= -1)
		{
			m_flNextEffectRem = chaos_effect_interval.GetFloat();
		}
		if (m_flNextEffectRem <= 0 && !pHL2Player->pl.deadflag)//don't start new effects when dead
		{
			engine->ClientCommand(engine->PEntityOfEntIndex(1), "sv_cheats 1");//always force cheats on to ensure everything works
			int nID = 0;
			// the order of weight functions can matter a lot. don't fuck it up
			if (!chaos_vote_enable.GetBool())
				nID = PickEffect(GetWeightSum());
			else
				nID = GetVoteWinnerEffect();

			RecoverWeights(0.2);
			
			//start effect
			StartGivenEffect(nID);
			PunishEffect(nID);
			if (chaos_vote_enable.GetBool())
				ResetVotes();
			if (pHL2Player->GetMoveType() != MOVETYPE_NOCLIP && chaos_unstuck_neweffect.GetBool())
				pHL2Player->GetUnstuck(500);
		}
	}
	else if (chaos_instant_off.GetBool()) // TODO: should be a command
	{
		//chaos was turned off
		for (int i = 0; i < NUM_EFFECTS; i++)
		{
			StopGivenEffect(i);
		}
		//some transient effects can technically last indefinitely (like spawning an npc)
		//some such effects (like spawning a weapon) are not worth the hassle of accurately tracking and disposing of. this code is more about function than form.
		CBaseEntity *pEnt = gEntList.FirstEnt();
		while (pEnt)
		{
			if (pEnt->m_bChaosSpawned || pEnt->m_bChaosPersist)
				pEnt->SUB_Remove();
			pEnt = gEntList.NextEnt(pEnt);
		}
	}
	float flTimeScale = cvar->FindVar("host_timescale")->GetFloat();
	if (!pHL2Player->pl.deadflag)
	{
		//you may think this is dumb, but the alternative is a bunch of arithmetic involving the effect durations, start times, and the curtime both before and after loading new states
		//and you must consider dying, miscellaneous fails, manual reloads, and level transitions, including transitions to prior levels.
		m_flNextEffectRem -= gpGlobals->interval_per_tick / flTimeScale;
	}
	for (int i = 0; i < m_activeEffects.Count(); i++)
	{
		CChaosEffect* effect = m_activeEffects[i];
		if (!pHL2Player->pl.deadflag) //don't progress timer when dead to avoid confusion
		{
			float effect_time_scale = effect->m_pTimeScaleCVar == nullptr ? 1.0f : effect->m_pTimeScaleCVar->GetFloat();
			effect->m_flTimeRem -= gpGlobals->interval_per_tick / flTimeScale / chaos_effect_time.GetFloat() / effect_time_scale;
			if (effect->m_flThinkDelay > 0.0f)
				effect->m_flThinkDelay -= gpGlobals->interval_per_tick / flTimeScale;
		}
		// TODO: we need to figure out how to move this work onto the engine, but this will work for now
		if (effect->m_flThinkDelay <= 0.0f)
		{
			effect->Think();
		}
		if (effect->m_flTimeRem <= 0 && !pHL2Player->pl.deadflag)//stop effects that are expiring, unless dead cause that's cheating
		{
			StopGivenEffect(effect);
		}
	}
	// TODO: this was being run every second
	// pHL2Player->MaintainEvils(); // TODO: this should be moved
}

ConVar chaos_strike_max("chaos_strike_max", "5", FCVAR_NONE, "Max number of times to allow an effect to kill player before ending it immediately");
void CChaosController::LevelInitPostEntity()
{
	if (gpGlobals->eLoadType == MapLoad_LoadGame) {
		//chaos_strike_max strikes, yer out
		for (int i = 0; i < m_activeEffects.Count(); i++)
		{
			CChaosEffect *pEffect = m_activeEffects[i];
			if (pEffect->m_iStrikes >= chaos_strike_max.GetInt())
			{
				Warning("Effect %s reached strike %i and was aborted\n", pEffect->GetName(), pEffect->m_iStrikes);
				StopGivenEffect(i);
			}
		}
		for (int i = 0; i < m_activeEffects.Count(); ++i)
		{
			CChaosEffect *pEffect = m_activeEffects[i];
			pEffect->RestoreEffect();
		}

	}
	if (gpGlobals->eLoadType == MapLoad_Transition) {
		for (int i = 0; i < m_activeEffects.Count(); ++i)
		{
			CChaosEffect *pEffect = m_activeEffects[i];
			pEffect->m_flThinkDelay = 0.0f; // TODO: this was in the old code. do we still need it?
			pEffect->LevelTransition();
		}
	}
}