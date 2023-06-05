#ifndef CHAOS_H
#define CHAOS_H
#pragma once

#include "cbase.h"
#include "utlpair.h"
#include "vehicle_jeep.h"

enum Effect_T
{
	EFFECT_ERROR,
	EFFECT_ZEROG,
	EFFECT_SUPERG,
	EFFECT_LOWG,
	EFFECT_INVERTG,
	EFFECT_PHYS_PAUSE,
	EFFECT_PHYS_FAST,
	EFFECT_PHYS_SLOW,
	EFFECT_PULL_TO_PLAYER,
	EFFECT_PUSH_FROM_PLAYER,
	EFFECT_NO_MOVEMENT,
	EFFECT_SUPER_MOVEMENT,
	EFFECT_LOCK_VEHICLE,
	EFFECT_NPC_HATE,
	EFFECT_NPC_LIKE,
	EFFECT_NPC_NEUTRAL,
	EFFECT_NPC_FEAR,
	EFFECT_TELEPORT_RANDOM,
	EFFECT_SPAWN_VEHICLE,
	EFFECT_SPAWN_NPC,
	EFFECT_SWIM_IN_AIR,
	EFFECT_ONLY_DRAW_WORLD,
	EFFECT_LOW_DETAIL,
	EFFECT_PLAYER_BIG,
	EFFECT_PLAYER_SMALL,
	EFFECT_NO_MOUSE_HORIZONTAL,
	EFFECT_NO_MOUSE_VERTICAL,
	EFFECT_SUPER_GRAB,
	EFFECT_GIVE_WEAPON,
	EFFECT_GIVE_ALL_WEAPONS,
	EFFECT_DROP_WEAPONS,
	//EFFECT_CROSSBOW_GUNS,//crossbow guns - replace bullets with crossbow bolts. requires moving some code around and i don't wanna deal with it right now
	EFFECT_NADE_GUNS,//grenade guns - replace bullets with grenades. same story as crossbow
	//rapid weapon fire - that will take a while to do
	EFFECT_EARTHQUAKE,
	//EFFECT_WINDY,//broken for unknown reasons
	EFFECT_FUNNY_NUMBER,
	EFFECT_ZOMBIE_SPAM,
	//EFFECT_LOW_FOV,
	//EFFECT_HIGH_FOV,
	EFFECT_EXPLODE_ON_DEATH,
	EFFECT_BULLET_TELEPORT,
	EFFECT_CREDITS,
	//EFFECT_SANTIAGO,
	EFFECT_SUPERHOT,
	EFFECT_SUPERCOLD,
	EFFECT_BARREL_SHOTGUN,
	EFFECT_QUICKCLIP_ON,
	EFFECT_QUICKCLIP_OFF,
	EFFECT_SOLID_TRIGGERS,
	EFFECT_RANDOM_COLORS,
	EFFECT_BEER_BOTTLE,
	// EFFECT_EVIL_ALYX,
	// EFFECT_EVIL_NORIKO,
	// EFFECT_CANT_LEAVE_MAP,
	// EFFECT_FLOOR_IS_LAVA,
	// EFFECT_PLAY_MUSIC,
	// EFFECT_USE_SPAM,
	// EFFECT_ORTHO_CAM,
	// EFFECT_FOREST,
	// EFFECT_SPAWN_MOUNTED_GUN,
	// EFFECT_RESTART_LEVEL,
	// EFFECT_REMOVE_PICKUPS,
	// EFFECT_CLONE_NPCS,
	// EFFECT_LOCK_PVS,
	// EFFECT_RELOAD_DEJA_VU,
	// EFFECT_BUMPY,
	// EFFECT_NO_BRAKE,
	// EFFECT_FORCE_INOUT_CAR,
	// EFFECT_WEAPON_REMOVE,
	// EFFECT_INTERP_NPCS,
	// EFFECT_PHYS_CONVERT,
	// EFFECT_INCLINE,
	// EFFECT_DISABLE_SAVE,
	// EFFECT_NO_RELOAD,
	// EFFECT_NPC_TELEPORT,
	// EFFECT_DEATH_WATER,
	// EFFECT_RANDOM_CC,

	NUM_EFFECTS
};

enum
{
	SPAWNTYPE_EYELEVEL_REGULAR,
	SPAWNTYPE_EYELEVEL_SPECIAL,
	SPAWNTYPE_CEILING,
	SPAWNTYPE_BIGFLYER,
	SPAWNTYPE_STRIDER,
	SPAWNTYPE_VEHICLE,
	SPAWNTYPE_UNDERGROUND,
};

// idea stolen from gamerules
// TODO: code duplication. this probably should be moved into shared
class CChaosControllerProxy : public CBaseEntity
{
public:
	DECLARE_CLASS(CChaosControllerProxy, CBaseEntity);
	DECLARE_SERVERCLASS();

	static void NotifyNetworkStateChanged();
	int UpdateTransmitState();
	static CChaosControllerProxy *s_pChaosControllerProxy;
};

class CChaosEffect
{
public:
	DECLARE_CLASS_NOBASE(CChaosEffect);
	DECLARE_SERVERCLASS_NOBASE();

	CChaosEffect(int EffectID, const char* Name, ConVar* WeightCVar, ConVar* TimeScaleCVar = nullptr);
	virtual void StartEffect(){};
	virtual void StopEffect(){};
	virtual void Think(){};
	virtual bool CheckStrike(const CTakeDamageInfo &info){ return false; };
	virtual bool CanBeChosen(){ return true; };

	virtual void LevelTransition(){};
	virtual void RestoreEffect(){};

	// TODO: this looks like an ugly hack and it should go away
	virtual void DoOnVehicles(CPropVehicleDriveable *pVehicle){};
	bool IterUsableVehicles(bool bFindOnly, bool bFindBoat = true, bool bFindBuggy = true);

	CAI_BaseNPC *ChaosSpawnNPC(const char *className, const char* strActualName, int iSpawnType, const char *strModel, const char *strTargetname, const char *strWeapon, bool bEvil = false);

	bool ChaosSpawnWeapon(const char *className, const char *strActualName, int iCount = 0, const char *strAmmoType = NULL, int iCount2 = 0, const char *strAmmoType2 = NULL);

	void SetNextThink(float delay);
	float GetSecondsRemaining();
	bool CheckEffectContext();
	bool DoRestorationAbort();

	const char* GetName();
	void SetName(const char* Name);

	// should be calling CChaosController but ehhhhhhhhhhhhhhh
	// This function is here for our CNetworkVars.
	inline void NetworkStateChanged()
	{
		// Forward the call to the entity that will send the data.
		CChaosControllerProxy::NotifyNetworkStateChanged();
	}

	inline void NetworkStateChanged( void *pVar )
	{
		// Forward the call to the entity that will send the data.
		CChaosControllerProxy::NotifyNetworkStateChanged();
	}

	CNetworkString( m_strName, MAX_EFFECT_NAME );

	// delay thinking for x seconds. NOTE: if not delayed in think, it will fire again next tick.
	// TODO: make -1 never think?
	float m_flThinkDelay;

	ConVar* m_pTimeScaleCVar;
	CNetworkVar( float, m_flTimeRem ); // 0-1 scale

	ConVar* m_pMaxWeightCVar;
	int m_iCurrentWeight;
	int m_iStrikes;

	int m_iExclude[NUM_EFFECTS];
	int m_iExcludeCount = 0;
	// DECLARE_SIMPLE_DATADESC();
};

class CChaosController : public CAutoGameSystemPerFrame
{
public:
	// DECLARE_CLASS_NOBASE(CChaosController)
	DECLARE_CLASS_GAMEROOT( CChaosController, CAutoGameSystemPerFrame );
	DECLARE_SERVERCLASS_NOBASE();

	CChaosController() : CAutoGameSystemPerFrame( "CChaosController" ) {};

	virtual bool Init();
	virtual void FrameUpdatePreEntityThink();
	virtual void LevelInitPostEntity();

	void Reset();

	void ResetVotes();
	int GetVoteWinnerEffect();

	int GetWeightSum();
	void RecoverWeights(float magnitute);
	void PunishEffect(int EffectID);

	int PickEffect(int iWeightSum);
	bool IsEffectActive(int EffectID);
	void StartGivenEffect(int EffectID);
	void StopGivenEffect(int EffectID);
	void StopGivenEffect(CChaosEffect* Effect);

	// This function is here for our CNetworkVars.
	inline void NetworkStateChanged()
	{
		// Forward the call to the entity that will send the data.
		CChaosControllerProxy::NotifyNetworkStateChanged();
	}

	inline void NetworkStateChanged( void *pVar )
	{
		// Forward the call to the entity that will send the data.
		CChaosControllerProxy::NotifyNetworkStateChanged();
	}

	CNetworkVar( float, m_flNextEffectRem );
	CChaosEffect* m_effects[NUM_EFFECTS]; // TODO: store entity factories instead of effect singletons?
	CUtlVector<CChaosEffect*> m_activeEffects;

	int m_iVoteNumber;
	// effect:votes
	CUtlVector<CUtlPair<int, int>> m_VoteOptions;
};

CChaosEffect* GetEffectByID(int effectID);

extern CChaosController g_chaosController;


//this is our macabre method of remembering persist entities. it's a holdover from when data was being stored in a txt file instead of global variables
//preferably this will be replaced with whatever point_template does
//i HIGHLY doubt storing the entire raw CBaseEntity would work well
class CChaosStoredEnt
{
public:
	//DECLARE_CLASS_NOBASE(CChaosStoredEnt);
	//string_t classname;
	const char *strClassname;
	string_t targetname;
	int chaosid;
	Vector origin;
	QAngle angle;
	int health;
	int max_health;
	int spawnflags;
	string_t model;
	int effects;
	int rendermode;
	int renderamt;
	Color rendercolor;
	int renderfx;
	int modelindex;
	float speed;
	int solid;
	//pKVData->SetInt("touchStamp", pEnt->touchStamp);//this was a speculative fix for some kind of touchlink related crash. if that crash comes back, put this back in.

	bool animating = false;
	int skin;
	int body;

	bool combatcharacter = false;

	bool npc = false;
	bool evil = false;

	bool hasweapon = false;
	string_t additionalequipment;

	bool headcrab = false;
	bool burrowed = false;
	bool hiding = false;
	bool ceiling = false;

	bool poisonzombie = false;
	bool crabs[3];
	int crabcount;

	bool antlion = false;
	//bool burrowed = false;

	bool antlionguard = false;
	//bool burrowed = false;
	bool cavernbreed = false;
};
//CUtlVector<int>				g_iActiveEffects;
// int g_iActiveEffects[MAX_ACTIVE_EFFECTS];
// CUtlVector<CChaosEffect *>	g_ChaosEffects;
extern int							g_iChaosSpawnCount;
extern CUtlVector<int>				g_iTerminated;//list of chaos ids to NOT restore from txt. used to remember which NPCs are dead as it would not make sense for them to come back to life.
extern CUtlVector<CChaosStoredEnt *> g_PersistEnts;
CChaosStoredEnt *StoreEnt(CBaseEntity *pEnt);
CBaseEntity *RetrieveStoredEnt(CChaosStoredEnt *pStoredEnt, bool bPersist);


extern bool						g_bGoBackLevel;
extern bool						g_bGroupsMade;

#define DEFINE_EFFECT_CONSTRUCTOR(classname, parent) \
	classname(int EffectID, const char* Name, ConVar* WeightCVar, ConVar* TimeScaleCVar = nullptr) \
	: parent(EffectID, Name, WeightCVar, TimeScaleCVar) {}

// TODO: steal get_timescale_cvar stuff or something from BEGIN_EFFECT
// #define DEFINE_EFFECT(classname, effectid, name, cvarname) \
// 	ConVar chaos_prob_##classname("chaos_prob_" #cvarname, "100"); \
// 	classname effect_##classname##_instance(effectid, MAKE_STRING(#name), &chaos_prob_##classname); 

#define BEGIN_EFFECT(effectid, name, cvarname, istimescaled) \
	namespace cvarname##_EffectHolder \
	{ \
		ConVar *get_weight_cvar() \
		{ \
			static ConVar weight_cvar_holder("chaos_prob_" #cvarname, "100");\
			return &weight_cvar_holder; \
		} \
		ConVar *get_timescale_cvar() \
		{ \
			if (istimescaled){ \
				static ConVar weight_cvar_holder("chaos_time_" #cvarname, "1");\
				return &weight_cvar_holder; \
			} else { \
				return nullptr; \
			} \
		} \
		const char* localname = name; \
		int localeffectid = effectid; \
		CChaosEffect *EffectCreate(); \
		CChaosEffect *g_EffectHolder = EffectCreate(); \
		class localEffect : public CChaosEffect \
		{ \
		public: \
			DEFINE_EFFECT_CONSTRUCTOR(localEffect, CChaosEffect);


#define END_EFFECT() \
		}; \
		CChaosEffect *EffectCreate() \
		{ \
			static localEffect effectHolder(localeffectid, localname, get_weight_cvar(), get_timescale_cvar()); \
			return &effectHolder; \
		} \
	}

#define DEFINE_EFFECT_SINGLEFIRE(effectid, name, cvarname) \
	BEGIN_EFFECT(effectid, name, cvarname, false) \
		void StartEffect(); \
	END_EFFECT() \
	void cvarname##_EffectHolder::localEffect::StartEffect()

// #define BEGIN_EFFECT_GUTS(effectid, name, classname, cvarname) \
// 	CChaosEffect *EffectCreate_##classname##_internal(); \
// 	namespace classname##_EffectHolder \
// 	{ \
// 		CChaosEffect *g_EffectHolder = EffectCreate_##classname##_internal(); \
// 	} \
// 	CChaosEffect *EffectCreate_##classname##_internal() \
// 	{ \
// 		static ConVar weight_cvar("chaos_prob_" #cvarname, "100"); \
// 		static string_t localname = MAKE_STRING(name); \
// 		static int localeffectid = effectid; \
// 		class localEffect : public CChaosEffect \
// 		{ \
// 		public: \
// 			DEFINE_EFFECT_CONSTRUCTOR(localEffect, CChaosEffect);

// #define END_EFFECT() \
// 		}; \
// 		static localEffect effectHolder(localeffectid, localname, &weight_cvar); \
// 		return &effectHolder; \
// 	}


// #define DEFINE_EFFECT_SINGLEFIRE(effectid, name, cvarname) \
// 	class cvarname##_EFFECT : public CChaosEffect \
// 	{ \
// 	public: \
// 		DEFINE_EFFECT_CONSTRUCTOR(cvarname##_EFFECT, CChaosEffect); \
// 		void StartEffect(); \
// 	}; \
// 	ConVar chaos_prob_##cvarname("chaos_prob_" #cvarname, "100"); \
// 	cvarname##_EFFECT effect_##cvarname##_instance(effectid, MAKE_STRING(name), &chaos_prob_##cvarname); \
// 	void cvarname##_EFFECT::StartEffect()

#endif
