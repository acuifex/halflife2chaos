#include "chaos_effect.h"

#include "chaos.h"

// let's be nice and make error as an effect
BEGIN_EFFECT(EFFECT_ERROR, "(ERROR)", error, false)
END_EFFECT()

ConVar chaos_beer_size_limit("chaos_beer_size_limit", "48", FCVAR_NONE, "Maximum size of beer bottle.");

DEFINE_EFFECT_SINGLEFIRE(EFFECT_BEER_BOTTLE, "Beer I owed ya", beer_bottle)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	QAngle vecAngles = QAngle(0, 0, 0);
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();
	CBaseAnimating *pEnt = (CBaseAnimating *)CreateEntityByName("prop_physics_override");
	pEnt->SetModel("models/props_junk/garbage_glassbottle003a.mdl");
	Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 128;
	trace_t	trDown;
	UTIL_TraceLine(vecOrigin, vecOrigin - Vector(0, 0, 1000), MASK_SOLID, pEnt, COLLISION_GROUP_NONE, &trDown);
	vecOrigin = trDown.endpos + Vector(0, 0, 1);
	Vector vecLastGoodPos = vecOrigin;
	pEnt->SetAbsOrigin(vecOrigin);
	trace_t	trace;
	int i = 0;
	//try to make bottle as big as possible with whatever space is in front of player
	do {
		/*
		pEnt->SetModelScale(i + 1);
		vecOrigin += Vector(0, 0, 9);//move bottle up a bit to account for the origin being above the ground
		vecOrigin += vecForward * 0.1 * i;//move bottle away as it gets bigger, or else we'll only get to about 45x before the trace fails because it's hitting the player
		Msg("i %i vecOrigin %0.1f %0.1f %0.1f\n", i, vecOrigin.x, vecOrigin.y, vecOrigin.z);
		//NDebugOverlay::Cross3D(vecOrigin, 16, 0, 255, 0, true, 30);
		UTIL_TraceEntity(pEnt, vecOrigin, vecOrigin, MASK_SOLID, &trace);
		i++;
		*/
		//NDebugOverlay::Cross3D(pEnt->GetAbsOrigin(), 16, 0, 255, 0, true, 30);
		pEnt->SetModelScale(i + 1);
		vecLastGoodPos = pEnt->GetAbsOrigin();
		pEnt->GetUnstuck(20, false);//only go up to 20 units away from previous position, instead of 500, which risks the beer spawning in some other random place, which is bad, i guess
		UTIL_TraceEntity(pEnt, pEnt->GetAbsOrigin(), pEnt->GetAbsOrigin(), MASK_SOLID, &trace);
		i++;
	} while (trace.fraction == 1 && !trace.startsolid && i < 55 && i < chaos_beer_size_limit.GetInt());//yes this i limit actually matters, or else we will create a beer so big it hits a max coord related assert and brings the whole game to a screeching halt. what the fuck.
	if (i > 1)
		pEnt->SetModelScale(i - 1);
	pEnt->SetMaxHealth(100 * i);
	pEnt->SetHealth(100 * i);
	DispatchSpawn(pEnt);
	pEnt->Activate();
	vecOrigin = vecLastGoodPos;
	pEnt->Teleport(&vecOrigin, &vecAngles, NULL);
	IPhysicsObject *pPhys = pEnt->VPhysicsGetObject();
	pPhys->EnableDrag(false);
	pPhys->SetMass(i * 10);
	if (i > 40)
		m_strName = MAKE_STRING("BEER I OWED YA");
	else
		m_strName = MAKE_STRING("Beer I owed ya"); // don't forget to change the text back, since effect object is persistent
}

// comments bellow are for quick reference while i'm reimplementing everything

#if 0
// CreateEffect<>(EFFECT_ERROR,							MAKE_STRING("(Error)"),						EC_NONE,								-1,											ERROR_WEIGHT);
// CreateEffect<CEGravitySet>(EFFECT_ZEROG,				MAKE_STRING("Zero Gravity"),				EC_NONE,								chaos_time_zerog.GetFloat(),				chaos_prob_zerog.GetInt());
// CreateEffect<CEGravitySet>(EFFECT_SUPERG,				MAKE_STRING("Super Gravity"),				EC_NONE,								chaos_time_superg.GetFloat(),				chaos_prob_superg.GetInt());
// CreateEffect<CEGravitySet>(EFFECT_LOWG,					MAKE_STRING("Low Gravity"),					EC_NONE,								chaos_time_lowg.GetFloat(),					chaos_prob_lowg.GetInt());
// CreateEffect<CEGravitySet>(EFFECT_INVERTG,				MAKE_STRING("Invert Gravity"),				EC_NONE,								chaos_time_invertg.GetFloat(),				chaos_prob_invertg.GetInt());
// CreateEffect<CEPhysSpeedSet>(EFFECT_PHYS_PAUSE,			MAKE_STRING("Pause Physics"),				EC_NO_VEHICLE,							chaos_time_phys_pause.GetFloat(),			chaos_prob_phys_pause.GetInt());
// CreateEffect<CEPhysSpeedSet>(EFFECT_PHYS_FAST,			MAKE_STRING("Fast Physics"),				EC_NONE,								chaos_time_phys_fast.GetFloat(),			chaos_prob_phys_fast.GetInt());
// CreateEffect<CEPhysSpeedSet>(EFFECT_PHYS_SLOW,			MAKE_STRING("Slow Physics"),				EC_NONE,								chaos_time_phys_slow.GetFloat(),			chaos_prob_phys_slow.GetInt());
// CreateEffect<CEPullToPlayer>(EFFECT_PULL_TO_PLAYER,		MAKE_STRING("Black Hole"),					EC_NONE,								chaos_time_pull_to_player.GetFloat(),		chaos_prob_pull_to_player.GetInt());
// CreateEffect<CEPushFromPlayer>(EFFECT_PUSH_FROM_PLAYER,	MAKE_STRING("Repulsive"),					EC_NONE,								chaos_time_push_from_player.GetFloat(),		chaos_prob_push_from_player.GetInt());
// CreateEffect<CEStop>(EFFECT_NO_MOVEMENT,				MAKE_STRING("Stop"),						EC_NONE,								chaos_time_no_movement.GetFloat(),			chaos_prob_no_movement.GetInt());
// CreateEffect<CESuperMovement>(EFFECT_SUPER_MOVEMENT,	MAKE_STRING("Super Speed"),					EC_NONE,								chaos_time_super_movement.GetFloat(),		chaos_prob_super_movement.GetInt());
// CreateEffect<CELockVehicles>(EFFECT_LOCK_VEHICLE,		MAKE_STRING("Lock Vehicles"),				EC_BOAT | EC_BUGGY,						chaos_time_lock_vehicle.GetFloat(),			chaos_prob_lock_vehicle.GetInt());
// CreateEffect<CENPCRels>(EFFECT_NPC_HATE,				MAKE_STRING("World of Hate"),				EC_NONE,								chaos_time_npc_hate.GetFloat(),				chaos_prob_npc_hate.GetInt());
// CreateEffect<CENPCRels>(EFFECT_NPC_LIKE,				MAKE_STRING("World of Love"),				EC_NONE,								chaos_time_npc_like.GetFloat(),				chaos_prob_npc_like.GetInt());
// CreateEffect<CENPCRels>(EFFECT_NPC_NEUTRAL,				MAKE_STRING("World of Apathy"),				EC_NONE,								chaos_time_npc_neutral.GetFloat(),			chaos_prob_npc_neutral.GetInt());
// CreateEffect<CENPCRels>(EFFECT_NPC_FEAR,				MAKE_STRING("World of Fear"),				EC_NONE,								chaos_time_npc_fear.GetFloat(),				chaos_prob_npc_fear.GetInt());
// CreateEffect<>(EFFECT_TELEPORT_RANDOM,					MAKE_STRING("Teleport to Random Place"),	EC_PLAYER_TELEPORT,						-1,											chaos_prob_teleport_random.GetInt());//
// CreateEffect<CERandomVehicle>(EFFECT_SPAWN_VEHICLE,		MAKE_STRING("Spawn Random Vehicle"),		EC_NONE,								-1,											chaos_prob_spawn_vehicle.GetInt());
// CreateEffect<CERandomNPC>(EFFECT_SPAWN_NPC,				MAKE_STRING("Spawn Random NPC"),			EC_NONE,								-1,											chaos_prob_spawn_npc.GetInt());
// CreateEffect<CESwimInAir>(EFFECT_SWIM_IN_AIR,			MAKE_STRING("Water World"),					EC_PICKUPS,								chaos_time_swim_in_air.GetFloat(),			chaos_prob_swim_in_air.GetInt());
// CreateEffect<>(EFFECT_ONLY_DRAW_WORLD,					MAKE_STRING("Where Are The Objects?"),		EC_NONE,								chaos_time_only_draw_world.GetFloat(),		chaos_prob_only_draw_world.GetInt());
// CreateEffect<>(EFFECT_LOW_DETAIL,						MAKE_STRING("Ultra Low Detail"),			EC_NONE,								chaos_time_low_detail.GetFloat(),			chaos_prob_low_detail.GetInt());
// CreateEffect<CEPlayerBig>(EFFECT_PLAYER_BIG,			MAKE_STRING("Player is Huge"),				EC_NONE,								chaos_time_player_big.GetFloat(),			chaos_prob_player_big.GetInt());
// CreateEffect<CEPlayerSmall>(EFFECT_PLAYER_SMALL,		MAKE_STRING("Player is Tiny"),				EC_NONE,								chaos_time_player_small.GetFloat(),			chaos_prob_player_small.GetInt());
// CreateEffect<>(EFFECT_NO_MOUSE_HORIZONTAL,				MAKE_STRING("No Looking Left/Right"),		EC_NONE,								chaos_time_no_mouse_horizontal.GetFloat(),	chaos_prob_no_mouse_horizontal.GetInt());
// CreateEffect<>(EFFECT_NO_MOUSE_VERTICAL,				MAKE_STRING("No Looking Up/Down"),			EC_NONE,								chaos_time_no_mouse_vertical.GetFloat(),	chaos_prob_no_mouse_vertical.GetInt());
// CreateEffect<CESuperGrab>(EFFECT_SUPER_GRAB,			MAKE_STRING("Didn't Skip Arm Day"),			EC_NONE,								chaos_time_super_grab.GetFloat(),			chaos_prob_super_grab.GetInt());
// CreateEffect<CERandomWeaponGive>(EFFECT_GIVE_WEAPON,	MAKE_STRING("Give Random Weapon"),			EC_NONE,								-1,											chaos_prob_give_weapon.GetInt());
// CreateEffect<>(EFFECT_GIVE_ALL_WEAPONS,					MAKE_STRING("Give All Weapons"),			EC_NONE,								-1,											chaos_prob_give_all_weapons.GetInt());
// CreateEffect<CEWeaponsDrop>(EFFECT_DROP_WEAPONS,		MAKE_STRING("Drop Weapons"),				EC_HAS_WEAPON | EC_NEED_PHYSGUN,		-1,											chaos_prob_drop_weapons.GetInt());
// CreateEffect<>(EFFECT_NADE_GUNS,						MAKE_STRING("Grenade Guns"),				EC_NO_INVULN,							chaos_time_nade_guns.GetFloat(),			chaos_prob_nade_guns.GetFloat());
// CreateEffect<CEEarthquake>(EFFECT_EARTHQUAKE,			MAKE_STRING("Wobbly"),						EC_NONE,								chaos_time_earthquake.GetFloat(),			chaos_prob_earthquake.GetInt());
// CreateEffect<CE420Joke>(EFFECT_420_JOKE,				MAKE_STRING("Funny Number"),				EC_NO_INVULN,							-1,											chaos_prob_420_joke.GetInt());
// CreateEffect<CEZombieSpam>(EFFECT_ZOMBIE_SPAM,			MAKE_STRING("Left 4 Dead"),					EC_HAS_WEAPON,							-1,											chaos_prob_zombie_spam.GetInt());
// CreateEffect<>(EFFECT_EXPLODE_ON_DEATH,					MAKE_STRING("NPCs Explode on Death"),		EC_NONE,								chaos_time_explode_on_death.GetFloat(),		chaos_prob_explode_on_death.GetInt());
// CreateEffect<>(EFFECT_BULLET_TELEPORT,					MAKE_STRING("Teleporter Bullets"),			EC_NONE,								chaos_time_bullet_teleport.GetFloat(),		chaos_prob_bullet_teleport.GetInt());
// CreateEffect<CECredits>(EFFECT_CREDITS,					MAKE_STRING("Credits"),						EC_NONE,								-1,											chaos_prob_credits.GetInt());
// CreateEffect<CESuperhot>(EFFECT_SUPERHOT,				MAKE_STRING("Superhot"),					EC_NONE,								chaos_time_superhot.GetFloat(),				chaos_prob_superhot.GetInt());
// CreateEffect<CESupercold>(EFFECT_SUPERCOLD,				MAKE_STRING("Supercold"),					EC_NONE,								chaos_time_supercold.GetFloat(),			chaos_prob_supercold.GetInt());
// CreateEffect<CEBarrelShotgun>(EFFECT_BARREL_SHOTGUN,	MAKE_STRING("Double Barrel Shotgun"),		EC_NONE,								chaos_time_barrel_shotgun.GetFloat(),		chaos_prob_barrel_shotgun.GetInt());
// CreateEffect<CEQuickclip>(EFFECT_QUICKCLIP_ON,			MAKE_STRING("Enable Quickclip"),			EC_QC_OFF,								chaos_time_quickclip_on.GetFloat(),			chaos_prob_quickclip_on.GetInt());
// CreateEffect<CEQuickclip>(EFFECT_QUICKCLIP_OFF,			MAKE_STRING("Disable Quickclip"),			EC_NONE,								chaos_time_quickclip_off.GetFloat(),		chaos_prob_quickclip_off.GetInt());
// CreateEffect<CESolidTriggers>(EFFECT_SOLID_TRIGGERS,	MAKE_STRING("Solid Triggers"),				EC_NONE,								chaos_time_solid_triggers.GetFloat(),		chaos_prob_solid_triggers.GetInt());
// CreateEffect<CEColors>(EFFECT_RANDOM_COLORS,			MAKE_STRING("Pretty Colors"),				EC_NONE,								chaos_time_random_colors.GetFloat(),		chaos_prob_random_colors.GetInt());
// CreateEffect<CEBottle>(EFFECT_BEER_BOTTLE,				MAKE_STRING("Beer I owed ya"),				EC_NONE,								-1,											chaos_prob_beer_bottle.GetInt());
// CreateEffect<CEEvilNPC>(EFFECT_EVIL_ALYX,				MAKE_STRING("Annoying Alyx"),				EC_HAS_WEAPON,							-1,											chaos_prob_evil_alyx.GetInt());
// CreateEffect<CEEvilNPC>(EFFECT_EVIL_NORIKO,				MAKE_STRING("Noriko, No!"),					EC_NONE,								-1,											chaos_prob_evil_noriko.GetInt());
// CreateEffect<>(EFFECT_CANT_LEAVE_MAP,					MAKE_STRING("Why So Rushed?"),				EC_NONE,								chaos_time_cant_leave_map.GetFloat(),		chaos_prob_cant_leave_map.GetInt());
// CreateEffect<CEFloorIsLava>(EFFECT_FLOOR_IS_LAVA,		MAKE_STRING("Floor Is Lava"),				EC_NO_INVULN | EC_QC_OFF,				chaos_time_floor_is_lava.GetFloat(),		chaos_prob_floor_is_lava.GetInt());
// CreateEffect<CERandomSong>(EFFECT_PLAY_MUSIC,			MAKE_STRING("Play Random Song"),			EC_NONE,								-1,											chaos_prob_play_music.GetInt());
// CreateEffect<CEUseSpam>(EFFECT_USE_SPAM,				MAKE_STRING("Grabby"),						EC_NO_VEHICLE,							chaos_time_use_spam.GetFloat(),				chaos_prob_use_spam.GetInt());
// CreateEffect<>(EFFECT_ORTHO_CAM,						MAKE_STRING("Orthographic Camera"),			EC_NONE,								chaos_time_ortho_cam.GetFloat(),			chaos_prob_ortho_cam.GetInt());
// CreateEffect<CETreeSpam>(EFFECT_FOREST,					MAKE_STRING("Surprise Reforestation!"),		EC_NONE,								chaos_time_forest.GetFloat(),				chaos_prob_forest.GetInt());
// CreateEffect<CEMountedGun>(EFFECT_SPAWN_MOUNTED_GUN,	MAKE_STRING("Spawn Mounted Gun"),			EC_NONE,								-1,											chaos_prob_spawn_mounted_gun.GetInt());
// CreateEffect<CERestartLevel>(EFFECT_RESTART_LEVEL,		MAKE_STRING("Restart Level"),				EC_NONE,								-1,											chaos_prob_restart_level.GetInt());
// CreateEffect<CERemovePickups>(EFFECT_REMOVE_PICKUPS,	MAKE_STRING("Remove All Pickups"),			EC_PICKUPS | EC_NEED_PHYSGUN | EC_HAS_WEAPON, -1,									chaos_prob_remove_pickups.GetInt());
// CreateEffect<CECloneNPCs>(EFFECT_CLONE_NPCS,			MAKE_STRING("Suppression Field Hiccup"),	EC_NONE,								-1,											chaos_prob_clone_npcs.GetInt());
// CreateEffect<CELockPVS>(EFFECT_LOCK_PVS,				MAKE_STRING("Vision Machine Broke"),		EC_NONE,								chaos_time_lock_pvs.GetFloat(),				chaos_prob_lock_pvs.GetInt());
// CreateEffect<CEDejaVu>(EFFECT_RELOAD_DEJA_VU,			MAKE_STRING("Deja Vu?"),					EC_PLAYER_TELEPORT,						-1,											chaos_prob_reload_deja_vu.GetInt());
// CreateEffect<CEBumpy>(EFFECT_BUMPY,						MAKE_STRING("Bumpy Road"),					EC_BUGGY,								chaos_time_bumpy.GetFloat(),				chaos_prob_bumpy.GetInt());
// CreateEffect<CENoBrake>(EFFECT_NO_BRAKE,				MAKE_STRING("Broken Brakes"),				EC_BUGGY,								chaos_time_no_brake.GetFloat(),				chaos_prob_no_brake.GetInt());
// CreateEffect<CEForceInOutCar>(EFFECT_FORCE_INOUT_CAR,	MAKE_STRING("Force In/Out Vehicle"),		EC_BUGGY | EC_BOAT | EC_PLAYER_TELEPORT,-1,											chaos_prob_force_inout_car.GetInt());
// CreateEffect<CEWeaponRemove>(EFFECT_WEAPON_REMOVE,		MAKE_STRING("Remove Random Weapon"),		EC_HAS_WEAPON | EC_NEED_PHYSGUN,		-1,											chaos_prob_weapon_remove.GetInt());
// CreateEffect<>(EFFECT_INTERP_NPCS,						MAKE_STRING("Laggy NPCs"),					EC_NONE,								chaos_time_interp_npcs.GetFloat(),			chaos_prob_interp_npcs.GetInt());
// CreateEffect<CEPhysConvert>(EFFECT_PHYS_CONVERT,		MAKE_STRING("Ran Out Of Glue"),				EC_NONE,								-1,											chaos_prob_phys_convert.GetInt());
// CreateEffect<CEIncline>(EFFECT_INCLINE,					MAKE_STRING("No Climbing"),					EC_NONE,								chaos_time_incline.GetFloat(),				chaos_prob_incline.GetInt());
// CreateEffect<>(EFFECT_DISABLE_SAVE,						MAKE_STRING("No Saving"),					EC_NONE,								chaos_time_disable_save.GetFloat(),			chaos_prob_disable_save.GetInt());
// CreateEffect<>(EFFECT_NO_RELOAD,						MAKE_STRING("No One Can Reload"),			EC_HAS_WEAPON,							chaos_time_no_reload.GetFloat(),			chaos_prob_no_reload.GetInt());
// CreateEffect<>(EFFECT_NPC_TELEPORT,						MAKE_STRING("You Teleport?"),				EC_NONE,								chaos_time_npc_teleport.GetFloat(),			chaos_prob_npc_teleport.GetInt());
// CreateEffect<CEDeathWater>(EFFECT_DEATH_WATER,			MAKE_STRING("Death Water"),					EC_WATER,								chaos_time_death_water.GetFloat(),			chaos_prob_death_water.GetInt());
// CreateEffect<CERandomCC>(EFFECT_RANDOM_CC,				MAKE_STRING("Color Incorrection"),			EC_NONE,								chaos_time_random_cc.GetFloat(),			chaos_prob_random_cc.GetInt());


bool CChaosEffect::CheckEffectContext()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	const char *pMapName = STRING(gpGlobals->mapname);

	if (chaos_ignore_context.GetBool())
		return true;

	//avoid long maps
	if (m_nID == EFFECT_RESTART_LEVEL)
		if (MapIsLong(pMapName))
			return false;//this is a long map

	//potential softlock if clone npcs happens on some maps
	if (m_nID == EFFECT_CLONE_NPCS)
		if (!Q_strcmp(pMapName, "d1_trainstation_01")	|| !Q_strcmp(pMapName, "d1_eli_01")			|| !Q_strcmp(pMapName, "d1_eli_02")			|| !Q_strcmp(pMapName, "d3_breen_01")
			|| !Q_strcmp(pMapName, "ep1_citadel_00")	|| !Q_strcmp(pMapName, "ep1_citadel_01")	|| !Q_strcmp(pMapName, "ep1_citadel_03")	|| !Q_strcmp(pMapName, "ep1_citadel_04") || !Q_strcmp(pMapName, "ep1_c17_00"))
			return false;

	//You Teleport is bad specifically on these maps
	if (m_nID == EFFECT_NPC_TELEPORT)
		if (!Q_strcmp(pMapName, "d1_trainstation_01") || !Q_strcmp(pMapName, "ep2_outland_12"))
			return false;//bad map

	//avoid maps that need striders or other NPCs to not teleport to god-knows-where
	//d3_c17_11 have to kill gunship
	//d3_c17_12b have to kill strider
	//d3_c17_13 have to kill striders
	//ep1_c17_00a alyx can become lost?
	//ep1_c17_05 have to kill sniper and APC
	//ep1_c17_06 have to kill strider
	//ep2_outland_01 alyx can become lost?
	//ep2_outland_08 have to kill helicopter
	//ep2_outland_12 have to kill striders
	if (m_nID == EFFECT_BULLET_TELEPORT)
		if (!Q_strcmp(pMapName, "d3_c17_11")			|| !Q_strcmp(pMapName, "d3_c17_12b")		|| !Q_strcmp(pMapName, "d3_c17_13")
			|| !Q_strcmp(pMapName, "ep1_c17_00a")		|| !Q_strcmp(pMapName, "ep1_c17_05")		|| !Q_strcmp(pMapName, "ep1_c17_06")
			|| !Q_strcmp(pMapName, "ep2_outland_01")	|| !Q_strcmp(pMapName, "ep2_outland_08")	|| !Q_strcmp(pMapName, "ep2_outland_12"))
			return false;

	//avoid maps that would poorly fit a crane
	if (m_nID == EFFECT_EVIL_NORIKO)
		if (!MapGoodForCrane(pMapName))
			return false;//map not good for cranes

	//quickclip must be on
	if (m_nID == EFFECT_QUICKCLIP_OFF)
		if (pPlayer->IsInAVehicle() || pPlayer->GetCollisionGroup() == COLLISION_GROUP_PLAYER)
			return false;//quickclip is off

	//need a shotgun ANYWHERE in the map, held or not
	if (m_nID == EFFECT_BARREL_SHOTGUN)
		if (gEntList.FindEntityByClassname(NULL, "weapon_sh*") == NULL)
			return false;//no sir no shotguns here

	//Don't remove pickups on these maps
	//d1_trainstation_05 suit to progress cutscene
	//d2_coast_10 rocket crate for gunship
	//d2_prison_01 rocket crate for gunships
	//d3_c17_09 grenades to save barney
	//d3_c17_10b explosives for ground turrets
	//d3_c17_11 rocket crate for gunship
	//d3_c17_13 rocket crate for striders
	//ep1_c17_00 pistol and shotgun to shoot lock
	//ep1_c17_05 rocket crate for sniper
	//ep1_c17_06 rocket crate for strider
	//ep2_outland_02 too hard
	//ep2_outland_09 grenades for autogun
	//ep2_outland_12 removing seems to break the respawn system?
	if (m_nID == EFFECT_REMOVE_PICKUPS)
		if (!Q_strcmp(pMapName, "d1_trainstation_05")
			|| !Q_strcmp(pMapName, "d2_coast_10")
			|| !Q_strcmp(pMapName, "d2_prison_01")
			|| !Q_strcmp(pMapName, "d3_c17_09")			|| !Q_strcmp(pMapName, "d3_c17_10b")		|| !Q_strcmp(pMapName, "d3_c17_11") || !Q_strcmp(pMapName, "d3_c17_13")
			|| !Q_strcmp(pMapName, "ep1_c17_00")		|| !Q_strcmp(pMapName, "ep1_c17_05")		|| !Q_strcmp(pMapName, "ep1_c17_06")
			|| !Q_strcmp(pMapName, "ep2_outland_02")	|| !Q_strcmp(pMapName, "ep2_outland_09")	|| !Q_strcmp(pMapName, "ep2_outland_12"))
			return false;

	//this is essentially just a list of all maps with elevators. quickclip will cause you to phase through elevators.
	if (m_nID == EFFECT_QUICKCLIP_ON)
		if (!Q_strcmp(pMapName, "d2_prison_05")			|| !Q_strcmp(pMapName, "d2_prison_06")		|| !Q_strcmp(pMapName, "d2_prison_08")
			|| !Q_strcmp(pMapName, "d3_citadel_03")		|| !Q_strcmp(pMapName, "d3_citadel_04")		|| !Q_strcmp(pMapName, "d3_breen_01")
			|| !Q_strcmp(pMapName, "ep1_citadel_01")	|| !Q_strcmp(pMapName, "ep1_citadel_03")	|| !Q_strcmp(pMapName, "ep1_c17_00a")
			|| !Q_strcmp(pMapName, "ep2_outland_12a")	|| !Q_strcmp(pMapName, "ep2_outland_03")	|| !Q_strcmp(pMapName, "ep2_outland_04"))
			return false;//bad map

	//Pause Physics can cause serious issues on these maps
	if (m_nID == EFFECT_PHYS_PAUSE)
		if (!Q_strcmp(pMapName, "d3_citadel_01")		|| !Q_strcmp(pMapName, "d3_citadel_02")		|| !Q_strcmp(pMapName, "d3_citadel_05")		|| !Q_strcmp(pMapName, "d3_breen_01")
			|| !Q_strcmp(pMapName, "ep2_outland_01")	|| !Q_strcmp(pMapName, "ep2_outland_03")	|| !Q_strcmp(pMapName, "ep2_outland_04")	|| !Q_strcmp(pMapName, "ep2_outland_11") || !Q_strcmp(pMapName, "ep2_outland_11b"))
			return false;//bad map

	//Ran Out Of Glue can cause serious issues on these maps
	if (m_nID == EFFECT_PHYS_CONVERT)
		if (!Q_strcmp(pMapName, "d1_trainstation_01")	|| !Q_strcmp(pMapName, "d1_trainstation_05")
			|| !Q_strcmp(pMapName, "d1_canals_11")		|| !Q_strcmp(pMapName, "d1_canals_13")
			|| !Q_strcmp(pMapName, "d1_eli_01")			|| !Q_strcmp(pMapName, "d1_town_01")
			|| !Q_strcmp(pMapName, "d3_c17_07")			|| !Q_strcmp(pMapName, "d3_c17_08")
			|| !Q_strcmp(pMapName, "d3_citadel_01")		|| !Q_strcmp(pMapName, "d3_citadel_02")		|| !Q_strcmp(pMapName, "d3_citadel_05")		|| !Q_strcmp(pMapName, "d3_breen_01")
			|| !Q_strcmp(pMapName, "ep1_citadel_03")	|| !Q_strcmp(pMapName, "ep1_c17_00a")
			|| !Q_strcmp(pMapName, "ep2_outland_01")	|| !Q_strcmp(pMapName, "ep2_outland_03")	|| !Q_strcmp(pMapName, "ep2_outland_11")	|| !Q_strcmp(pMapName, "ep2_outland_11b"))
			return false;//bad map

	//could distrupt cutscenes
	if (m_nID == EFFECT_NPC_HATE || m_nID == EFFECT_NPC_FEAR)
		if (!Q_strcmp(pMapName, "d1_trainstation_04") || !Q_strcmp(pMapName, "d1_canals_03") || !Q_strcmp(pMapName, "d1_eli_01")
			|| !Q_strcmp(pMapName, "d2_coast_10")
			|| !Q_strcmp(pMapName, "d3_breen_01")
			|| !Q_strcmp(pMapName, "ep1_citadel_03") || !Q_strcmp(pMapName, "ep1_c17_02b")
			|| !Q_strcmp(pMapName, "ep2_outland_01") || !Q_strcmp(pMapName, "ep2_outland_07") || !Q_strcmp(pMapName, "ep2_outland_08") || !Q_strcmp(pMapName, "ep2_outland_10a"))
			return false;//bad map

	//if we miss the trigger_changelevel, the pod will get killed, killing us
	if (m_nID == EFFECT_CANT_LEAVE_MAP)
		if (!Q_strcmp(pMapName, "d3_citadel_01"))
			return false;

	//on this map, zombies could spawn at just the right time that alyx will be scripted to stand in the control room,
	//while the player is unable to reach it in a reasonable time to save alyx as she is scripted to stand in the room
	if (m_nID == EFFECT_ZOMBIE_SPAM)
		if (!Q_strcmp(pMapName, "ep1_citadel_03"))
			return false;

	if (m_nContext == EC_NONE)
		return true;

	//need at least one vehicle of desired type
	if (m_nContext & EC_BUGGY || m_nContext & EC_BOAT)
		if (!IterUsableVehicles(true))
			return false;//found no desired vehicles

	//don't use when usable vehicles are around
	if (m_nContext & EC_NO_VEHICLE)
		if (IterUsableVehicles(true))
			return false;//vehicle found

	//need at least one pickup in the map
	if (m_nContext & EC_PICKUPS)
		if (pPlayer->GetHealth() < 50 || gEntList.FindEntityByClassname(NULL, "it*") == NULL)
			return false;//no pickups

	//quickclip must be off
	//TODO: is the player's collision group ever anything other than these two?
	if (m_nContext & EC_QC_OFF)
	{
		if (!pPlayer->IsInAVehicle() && pPlayer->GetCollisionGroup() == COLLISION_GROUP_IN_VEHICLE)
			return false;//quickclip is on
		if (pPlayer->IsInAVehicle())
			return false;//can't turn quickclip on when you're in a vehicle
	}

	//need water in the map
	if (m_nContext & EC_WATER)
		if (gEntList.FindEntityByClassname(NULL, "wa*") == NULL)
			return false;//no water in map

	//player must have a weapon
	//also must not be in a normal-gravity-gun-only map
	if (m_nContext & EC_HAS_WEAPON)
		if (pPlayer->GetActiveWeapon() == NULL || !Q_strcmp(pMapName, "ep1_citadel_00") || !Q_strcmp(pMapName, "ep1_citadel_01") || !Q_strcmp(pMapName, "ep2_outland_01"))
			return false;//player has no weapons

	//player must NOT be invulnerable
	if (m_nContext & EC_NO_INVULN)
	{
		if (GlobalEntity_GetState("gordon_invulnerable") == GLOBAL_ON)
			return false;//player is invulnerable
		//maybe we have a damage filter set
		if (pPlayer->m_hDamageFilter)
		{
			CBaseFilter *pFilter = (CBaseFilter *)(!pPlayer->m_hDamageFilter.Get());
			CFilterClass *pClassFilter = dynamic_cast<CFilterClass*>(pFilter);
			if (pClassFilter && !Q_strcmp(STRING(pClassFilter->m_iFilterClass), "!invulnerable"))
				return false;//invulnerable via damage filter
		}
	}

	//this effect could permanently separate us from the gravity gun at a time where we need it
	if (m_nContext & EC_NEED_PHYSGUN)
		if (GlobalEntity_GetState("super_phys_gun") == GLOBAL_ON
			|| !Q_strcmp(pMapName, "d3_c17_07")			|| !Q_strcmp(pMapName, "d3_c17_08")			|| !Q_strcmp(pMapName, "d3_c17_10b")
			|| !Q_strcmp(pMapName, "d3_citadel_03")		|| !Q_strcmp(pMapName, "d3_citadel_04")		|| !Q_strcmp(pMapName, "d3_breen_01")
			|| !Q_strcmp(pMapName, "ep1_citadel_00")	|| !Q_strcmp(pMapName, "ep1_citadel_01")	|| !Q_strcmp(pMapName, "ep1_citadel_03")	|| !Q_strcmp(pMapName, "ep1_citadel_04")
			|| !Q_strcmp(pMapName, "ep1_c17_00")		|| !Q_strcmp(pMapName, "ep1_c17_00a")		|| !Q_strcmp(pMapName, "ep1_c17_01")		|| !Q_strcmp(pMapName, "ep1_c17_02"))
			return false;//bad time to lose the gravity gun

	//NO TELEPORT LIST LEAKED
	if (m_nContext & EC_PLAYER_TELEPORT)
	{
		//don't need to check map list if we would be forcing out of a car, cause we don't teleport (far) in that case, unless you did the ladder bug thing
		if (!(m_nID == EFFECT_FORCE_INOUT_CAR && pPlayer->IsInAVehicle()))
		{
			if (!Q_strcmp(pMapName, "d1_trainstation_01")	|| !Q_strcmp(pMapName, "d1_trainstation_04")|| !Q_strcmp(pMapName, "d1_trainstation_05")
				|| !Q_strcmp(pMapName, "d1_canals_01")		|| !Q_strcmp(pMapName, "d1_canals_05")		|| !Q_strcmp(pMapName, "d1_canals_06")		|| !Q_strcmp(pMapName, "d1_canals_08")		|| !Q_strcmp(pMapName, "d1_canals_11")
				|| !Q_strcmp(pMapName, "d1_eli_01")			|| !Q_strcmp(pMapName, "d1_eli_02")
				|| !Q_strcmp(pMapName, "d1_town_02a")		|| !Q_strcmp(pMapName, "d1_town_05")
				|| !Q_strcmp(pMapName, "d2_coast_11")
				|| !Q_strcmp(pMapName, "d2_prison_06")		|| !Q_strcmp(pMapName, "d2_prison_08")
				|| !Q_strcmp(pMapName, "d3_c17_06b")		|| !Q_strcmp(pMapName, "d3_c17_07")			|| !Q_strcmp(pMapName, "d3_c17_10b")		|| !Q_strcmp(pMapName, "d3_c17_13")
				|| !Q_strcmp(pMapName, "d3_citadel_03")		|| !Q_strcmp(pMapName, "d3_citadel_04")
				|| !Q_strcmp(pMapName, "d3_breen_01")
				|| !Q_strcmp(pMapName, "ep1_citadel_01")	|| !Q_strcmp(pMapName, "ep1_citadel_03")	|| !Q_strcmp(pMapName, "ep1_citadel_04")
				|| !Q_strcmp(pMapName, "ep1_c17_00")		|| !Q_strcmp(pMapName, "ep1_c17_00a")
				|| !Q_strcmp(pMapName, "ep2_outland_01")	|| !Q_strcmp(pMapName, "ep2_outland_03")	|| !Q_strcmp(pMapName, "ep2_outland_06a")	|| !Q_strcmp(pMapName, "ep2_outland_09")
				|| !Q_strcmp(pMapName, "ep2_outland_10")	|| !Q_strcmp(pMapName, "ep2_outland_11")	|| !Q_strcmp(pMapName, "ep2_outland_11a")	|| !Q_strcmp(pMapName, "ep2_outland_11b")	|| !Q_strcmp(pMapName, "ep2_outland_12") || !Q_strcmp(pMapName, "ep2_outland_12a"))
				return false;//no
		}
	}

	//you did it
	return true;
}
class CEBumpy : public CChaosEffect
{
public:
	void FastThink() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
	bool m_bReverse;
};
class CECloneNPCs : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CERemovePickups : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CERestartLevel : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEMountedGun : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CETreeSpam : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CERandomSong : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEEvilNPC : public CChaosEffect
{
public:
	void StartEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
	void EvilNoriko();
	int m_iSavedChaosID;
};
class CEBottle : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEColors : public CChaosEffect
{
public:
	void StartEffect() override;
	void MaintainEffect() override;
};
class CEPushFromPlayer : public CChaosEffect
{
public:
	void MaintainEffect() override;
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
};
class CEPullToPlayer : public CChaosEffect
{
public:
	void MaintainEffect() override;
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CESuperhot : public CChaosEffect
{
public:
	void FastThink() override;
};
class CESupercold : public CChaosEffect
{
public:
	void FastThink() override;
};
class CECredits : public CChaosEffect
{
public:
	void StartEffect() override;
	void MaintainEffect() override;
	void RestoreEffect() override;
	void TransitionEffect() override;
	bool m_bPlayedSecondSong = false;
};
class CESolidTriggers : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
};
class CESuperMovement : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CELockVehicles : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
};
class CERandomNPC : public CChaosEffect
{
public:
	void StartEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
	int m_iSavedChaosID;
	void MaintainEffect() override;
};
class CERandomVehicle : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CERandomWeaponGive : public CChaosEffect
{
public:
	void StartEffect() override;
};
//intentionally no strike check because the chance that you can't kill at least one zombie per life is super low
//maybe we could do something but ehhhh. the prospect of dying and reloading from this effect is part of the appeal.
//maybe instead of abort removing all zombies, just remove the one that did the final blow
class CEZombieSpam : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CENPCRels : public CChaosEffect
{
public:
	void DoNPCRels(int disposition, bool bRevert);
	void StartEffect() override;
	void StopEffect() override;
	void MaintainEffect() override;
};
class CEGravitySet : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
	void MaintainEffect() override;
};
class CEPhysSpeedSet : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEStop : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
};
class CESwimInAir : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void FastThink() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CELockPVS : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
};
class CEPlayerBig : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void MaintainEffect() override;
};
class CEPlayerSmall : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void MaintainEffect() override;
};
class CESuperGrab : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEWeaponsDrop : public CChaosEffect
{
public:
	void StartEffect() override;
	void FastThink() override;
	bool m_bDone = false;
};
class CEEarthquake : public CChaosEffect
{
public:
	void StartEffect() override;
	void TransitionEffect() override;
};
class CE420Joke : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEQuickclip : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEFloorIsLava : public CChaosEffect
{
public:
	void FastThink() override;
	int m_iSkipTicks = 0;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CEUseSpam : public CChaosEffect
{
public:
	void MaintainEffect() override;
	float m_flLastUseThink = -1;
};
class CENoBrake : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEForceInOutCar : public CChaosEffect
{
public:
	void StartEffect() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
	bool m_bFoundOne = false;
};
class CEWeaponRemove : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEPhysConvert : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEIncline : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEDeathWater : public CChaosEffect
{
public:
	void FastThink() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CEBarrelShotgun : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CEDejaVu : public CChaosEffect
{
public:
	void StartEffect() override;
	void MaintainEffect() override;
	bool m_bDone = false;
};
class CERandomCC : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
	void RestoreEffect() override;
};

void CChaosEffect::StartEffect()
{
	//all code related to timing etc intentionally left out of here because StartEffect is called from less common places as well
	switch (m_nID)
	{
	case EFFECT_TELEPORT_RANDOM:
		RandomTeleport(true);
		break;
	case EFFECT_ONLY_DRAW_WORLD:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_drawfuncdetail 0;r_drawstaticprops 0;r_drawentities 0");
		break;
	case EFFECT_LOW_DETAIL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "mat_picmip 4;r_lod 6;mat_filtertextures 0;mat_filterlightmaps 0");
		break;
	case EFFECT_NO_MOUSE_HORIZONTAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "sv_cheats 1;wait 10;m_yaw 0.0f;cl_yawspeed 0");
		break;
	case EFFECT_NO_MOUSE_VERTICAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "sv_cheats 1;wait 10;m_pitch 0.0f;cl_pitchspeed 0");
		break;
	case EFFECT_GIVE_ALL_WEAPONS:
		UTIL_GetLocalPlayer()->EquipSuit();
		ChaosSpawnWeapon("weapon_crowbar", MAKE_STRING("Give All Weapons"));
		ChaosSpawnWeapon("weapon_physcannon", MAKE_STRING("Give All Weapons"));
		ChaosSpawnWeapon("weapon_pistol", MAKE_STRING("Give All Weapons"), 75, "Pistol");
		ChaosSpawnWeapon("weapon_357", MAKE_STRING("Give All Weapons"), 9, "357");
		ChaosSpawnWeapon("weapon_smg1", MAKE_STRING("Give All Weapons"), 128, "SMG1", 1, "smg1_grenade");
		ChaosSpawnWeapon("weapon_ar2", MAKE_STRING("Give All Weapons"), 30, "AR2", 1, "AR2AltFire");
		ChaosSpawnWeapon("weapon_shotgun", MAKE_STRING("Give All Weapons"), 15, "Buckshot");
		ChaosSpawnWeapon("weapon_crossbow", MAKE_STRING("Give All Weapons"), 5, "XBowBolt");
		ChaosSpawnWeapon("weapon_frag", MAKE_STRING("Give All Weapons"), 5, "grenade");
		ChaosSpawnWeapon("weapon_rpg", MAKE_STRING("Give All Weapons"), 3, "rpg_round");
		ChaosSpawnWeapon("weapon_bugbait", MAKE_STRING("Give All Weapons"));
		GlobalEntity_Add(MAKE_STRING("antlion_allied"), gpGlobals->mapname, GLOBAL_ON);//antlions become friendly
		break;
	case EFFECT_NADE_GUNS:
		chaos_replace_bullets_with_grenades.SetValue(1);
		break;
	case EFFECT_EXPLODE_ON_DEATH:
		chaos_explode_on_death.SetValue(1);
		break;
	case EFFECT_BULLET_TELEPORT:
		chaos_bullet_teleport.SetValue(1);
		break;
	case EFFECT_CANT_LEAVE_MAP:
		chaos_cant_leave_map.SetValue(1);
		break;
	case EFFECT_ORTHO_CAM:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "camortho;c_orthoheight 135;c_orthowidth 240\n");
		break;
	case EFFECT_INTERP_NPCS:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "cl_interp_npcs 5");
		break;
	case EFFECT_DISABLE_SAVE:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "save_disable 1");
		break;
	case EFFECT_NO_RELOAD:
		chaos_no_reload.SetValue(1);
		break;
	case EFFECT_NPC_TELEPORT:
		chaos_npc_teleport.SetValue(1);
		break;
	}
}// StartEffect()
void CChaosEffect::StopEffect()
{
	switch (m_nID)
	{
	case EFFECT_ONLY_DRAW_WORLD:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_drawfuncdetail 1;r_drawstaticprops 1;r_drawentities 1\n");
		break;
	case EFFECT_LOW_DETAIL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "mat_picmip -1;r_lod -1;mat_filtertextures 1;mat_filterlightmaps 1\n");
		break;
	case EFFECT_NO_MOUSE_HORIZONTAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "exec yaw\n");
		break;
	case EFFECT_NO_MOUSE_VERTICAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "exec pitch\n");
		break;
	case EFFECT_NADE_GUNS:
		chaos_replace_bullets_with_grenades.SetValue(0);
		break;
	case EFFECT_EXPLODE_ON_DEATH:
		chaos_explode_on_death.SetValue(0);
		break;
	case EFFECT_BULLET_TELEPORT:
		chaos_bullet_teleport.SetValue(0);
		break;
	case EFFECT_SUPERHOT:
	case EFFECT_SUPERCOLD:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "host_timescale 1");
		break;
	case EFFECT_CANT_LEAVE_MAP:
		chaos_cant_leave_map.SetValue(0);
		break;
	case EFFECT_ORTHO_CAM:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "camnormal\n");
		break;
	case EFFECT_INTERP_NPCS:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "cl_interp_npcs 0");
		break;
	case EFFECT_DISABLE_SAVE:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "save_disable 0");
		break;
	case EFFECT_NO_RELOAD:
		chaos_no_reload.SetValue(0);
		break;
	case EFFECT_NPC_TELEPORT:
		chaos_npc_teleport.SetValue(0);
		break;
	}
}// StopEffect()

//
//Make sure all effects are present on loading save.
//
void CChaosEffect::RestoreEffect()
{
	Msg("Restoring effect %i\n", m_nID);
	StartEffect();
}

//Do not restore:
//Simple convar changes or any other thing that isn't affected by world state.
//Transient things, unless they have an override RestoreEffect. If spawned entities wish to persist through saves they have their own thing.
//Effects that can survive on MaintainEffect or FastThink.
bool CChaosEffect::DoRestorationAbort()
{
	switch (m_nID)
	{
	//env_physexplosion has be recreated or deleted
	case EFFECT_PULL_TO_PLAYER:
	case EFFECT_PUSH_FROM_PLAYER:

	//trees have to be recreated or deleted
	case EFFECT_FOREST:

	//alters vehicles
	case EFFECT_NO_MOVEMENT:
	case EFFECT_LOCK_VEHICLE:
	case EFFECT_BUMPY:

	//alters NPCs
	case EFFECT_NPC_HATE:
	case EFFECT_NPC_LIKE:
	case EFFECT_NPC_NEUTRAL:
	case EFFECT_NPC_FEAR:

	//alters player 
	case EFFECT_PLAYER_BIG://CBaseAnimating::m_flModelScale
	case EFFECT_PLAYER_SMALL://CBaseAnimating::m_flModelScale
	case EFFECT_SUPER_GRAB://CBasePlayer::m_bSuperGrab
	case EFFECT_SUPER_MOVEMENT://CBasePlayer::m_flMaxspeed
	case EFFECT_INCLINE://CPlayerLocalData::m_flStepSize
	case EFFECT_SWIM_IN_AIR://CBasePlayer::m_bSwimInAir
	case EFFECT_QUICKCLIP_ON://collision group
	case EFFECT_QUICKCLIP_OFF://collision group

	//alters triggers
	case EFFECT_SOLID_TRIGGERS:

	//gone forever if not restarted
	case EFFECT_EARTHQUAKE:

	//see CECredits::RestoreEffect()
	case EFFECT_CREDITS:

	//if r_lockpvs is 1 when reloading, there is nothing visible at all
	case EFFECT_LOCK_PVS:
		return true;
	}
	return false;
}

void CChaosEffect::RandomTeleport(bool bPlayerOnly)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	//We could use a more advanced method in the future to allow teleporting to even more places than this allows, but for now, this is good enough
	//teleport to a random node
	int nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, g_pBigAINet->NumNodes() - 1) : chaos_rng1.GetInt();
	CAI_Node *pNode = g_pBigAINet->GetNode(nRandom);
	if ( !pNode )
		return; // Some maps may be lacking node graphs
	Vector vec = pNode->GetPosition(HULL_HUMAN);
	if (pPlayer->GetVehicle() && pPlayer->GetVehicle()->GetVehicleEnt())
	{
		vec += Vector(0, 0, 64);
		pPlayer->GetVehicle()->GetVehicleEnt()->Teleport(&vec, NULL, NULL);
		pPlayer->GetVehicle()->GetVehicleEnt()->GetUnstuck(500);
	}
	else
	{
		if (pPlayer->GetMoveType() == MOVETYPE_NONE)
		{
			g_GameMovement.GetLadderMove()->m_bForceLadderMove = false;
			pPlayer->SetMoveType(MOVETYPE_WALK);
		}
		pPlayer->Teleport(&vec, NULL, NULL);
		pPlayer->GetUnstuck(500);
	}
}
CBaseEntity *CChaosEffect::ChaosSpawnVehicle(const char *className, string_t strActualName, int iSpawnType, const char *strModel, const char *strTargetname, const char *strScript)
{
	float flDistAway;
	float flExtraHeight;
	switch (iSpawnType)
	{
	case SPAWNTYPE_EYELEVEL_SPECIAL:
		flDistAway = 128;
		flExtraHeight = 64;
		break;
	case SPAWNTYPE_VEHICLE:
		flDistAway = 256;//don't bonk player on head
		flExtraHeight = 32;
		break;
	default:
		return NULL;
	}
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	//CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();//This will always give back some actual XY numbers because the camera is actually limited to 89 degrees up or down, not 90
	//then again something weird COULD happen to set the angle to straight 90 up/down, but idk what that would be
	CBaseEntity *pVehicle = (CBaseEntity *)CreateEntityByName(className);
	if (pVehicle)
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * flDistAway + Vector(0, 0, flExtraHeight);

		//see if we're looking at a wall
		trace_t tr;
		UTIL_TraceLine(pPlayer->GetAbsOrigin() + Vector(0, 0, flExtraHeight), vecOrigin, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
		//push out of wall. GetUnstuck doesn't always work because UTIL_TraceEntity is shit
		if (tr.fraction != 1.0 && !FStrEq(className, "prop_vehicle_crane"))//crane is generally immovable, take NO risk in spawning intersecting player
			vecOrigin = tr.endpos - vecForward * (70);
		QAngle vecAngles(0, pPlayer->GetAbsAngles().y, 0);
		pVehicle->SetAbsOrigin(vecOrigin);
		pVehicle->SetAbsAngles(vecAngles);
		if (FStrEq(className, "prop_vehicle_apc"))//APCs aren't set up to be driveable
			pVehicle->KeyValue("VehicleLocked", "1");
		pVehicle->KeyValue("model", strModel);
		pVehicle->KeyValue("solid", "6");
		pVehicle->KeyValue("targetname", strTargetname);
		pVehicle->KeyValue("EnableGun", "1");
		pVehicle->KeyValue("CargoVisible", "1");
		pVehicle->KeyValue("vehiclescript", strScript);
		g_iChaosSpawnCount++; pVehicle->KeyValue("chaosid", g_iChaosSpawnCount);
		DispatchSpawn(pVehicle);
		pVehicle->Activate();
		pVehicle->Teleport(&vecOrigin, &vecAngles, NULL);
		m_strHudName = strActualName;
		trace_t	trace;
		UTIL_TraceEntity(pVehicle, vecOrigin, vecOrigin, MASK_SOLID, &trace);
		if (trace.startsolid)
			pVehicle->GetUnstuck(500);
	}
	return pVehicle;
}
bool CChaosEffect::ChaosSpawnWeapon(const char *className, string_t strActualName, int iCount, const char *strAmmoType, int iCount2, const char *strAmmoType2)
{
	bool bGaveWeapon = UTIL_GetLocalPlayer()->GiveNamedItem(className) != NULL;
	if (!bGaveWeapon)
		return false;
	m_strHudName = strActualName;
	if (iCount)
		UTIL_GetLocalPlayer()->GiveAmmo(iCount, strAmmoType);
	if (iCount2)
		UTIL_GetLocalPlayer()->GiveAmmo(iCount2, strAmmoType2);
	return bGaveWeapon;
}
ConVar getnearbynodes_debug("getnearbynodes_debug", "0");
CNodeList *CChaosEffect::GetNearbyNodes(int iNodes)
{
	CAI_Node *pNode;
	float flClosest = FLT_MAX;
	bool full = false;
	CNodeList *result = new CNodeList;
	result->SetLessFunc(CNodeList::RevIsLowerPriority);//this impacts sorting, MUST be kept
	for (int node = 0; node < g_pBigAINet->NumNodes(); node++)
	{
		pNode = g_pBigAINet->GetNode(node);
		if (pNode->GetType() != NODE_GROUND)
		{
			if (getnearbynodes_debug.GetBool()) Msg("Rejected node %i for not being a ground node\n", pNode->GetId());
			continue;
		}
		float flDist = (UTIL_GetLocalPlayer()->GetAbsOrigin() - pNode->GetPosition(HULL_HUMAN)).Length();
		if (flDist < flClosest)
		{
			if (getnearbynodes_debug.GetBool()) Msg("node %i is closer (%0.1f) than previous closest (%0.1f)\n", pNode->GetId(), flDist, flClosest);
			flClosest = flDist;
		}
		if (!full || (flDist < result->ElementAtHead().dist))
		{
			if (getnearbynodes_debug.GetBool()) Msg("Adding node %i to list. full is %s, %0.1f < %0.1f\n", pNode->GetId(), full ? "TRUE" : "FALSE", flDist, result->Count() > 0 ? result->ElementAtHead().dist : 1234);
			if (full)
			{
				if (getnearbynodes_debug.GetBool()) Msg("List full, removing node %i to add node %i\n", result->ElementAtHead().nodeIndex, pNode->GetId());
				result->RemoveAtHead();
			}
			result->Insert(AI_NearNode_t(node, flDist));
			full = (result->Count() == iNodes);
		}
		else if (flDist >= result->ElementAtHead().dist)
		{
			if (getnearbynodes_debug.GetBool()) Warning("Not adding  %i to list. full is %s, %0.1f < %0.1f\n", pNode->GetId(), full ? "TRUE" : "FALSE", flDist, result->Count() > 0 ? result->ElementAtHead().dist : 1234);
		}
	}
	Msg("list has %i nodes\n", result->Count());
	return result;
}
CAI_Node *CChaosEffect::NearestNodeToPoint(const Vector &vPosition, bool bCheckVisibility)
{
	return g_pBigAINet->GetNode(g_pBigAINet->NearestNodeToPoint(vPosition, bCheckVisibility), false);
}
bool CChaosEffect::IterUsableVehicles(bool bFindOnly)
{
	bool bFoundSomething = false;
	bool bFindBoat = m_nContext & EC_BOAT || m_nContext & EC_NO_VEHICLE || m_nContext == EC_NONE;
	bool bFindBuggy = m_nContext & EC_BUGGY || m_nContext & EC_NO_VEHICLE || m_nContext == EC_NONE;
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
CBaseEntity *CChaosEffect::GetEntityWithID(int iChaosID)
{
	CBaseEntity *pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		if (pEnt->m_iChaosID == iChaosID)
			return pEnt;
		pEnt = gEntList.NextEnt(pEnt);
	}
	return NULL;
}

bool CChaosEffect::MapIsLong(const char *pMapName)
{
	return !Q_strcmp(pMapName, "d1_trainstation_01")	|| !Q_strcmp(pMapName, "d1_trainstation_05")
		|| !Q_strcmp(pMapName, "d1_eli_01")				|| !Q_strcmp(pMapName, "d1_eli_02")
		|| !Q_strcmp(pMapName, "d1_town_02")
		|| !Q_strcmp(pMapName, "d2_coast_07")			|| !Q_strcmp(pMapName, "d2_coast_08")
		|| !Q_strcmp(pMapName, "d2_prison_05")			|| !Q_strcmp(pMapName, "d2_prison_06")				|| !Q_strcmp(pMapName, "d2_prison_07")			|| !Q_strcmp(pMapName, "d2_prison_08")
		|| !Q_strcmp(pMapName, "d3_c17_13")				|| !Q_strcmp(pMapName, "d3_citadel_02")				|| !Q_strcmp(pMapName, "d3_citadel_05")			|| !Q_strcmp(pMapName, "d3_breen_01")

		|| !Q_strcmp(pMapName, "ep1_citadel_00")		|| !Q_strcmp(pMapName, "ep1_citadel_01")			|| !Q_strcmp(pMapName, "ep1_citadel_03")

		|| !Q_strcmp(pMapName, "ep2_outland_02")		|| !Q_strcmp(pMapName, "ep2_outland_11")			|| !Q_strcmp(pMapName, "ep2_outland_11b")		|| !Q_strcmp(pMapName, "ep2_outland_12")	|| !Q_strcmp(pMapName, "ep2_outland_12a");
}

//Disallow maps that wouldn't let a crane do much. cranes need wide open land to graze on.
//This is a blocklist because an allowlist would restrict all third party maps
//plus it's less string comparisons this way
bool CChaosEffect::MapGoodForCrane(const char *pMapName)
{
	//trigger_physics_trap can cause the magnet to become vaporized
	CBaseEntity *pRemover = gEntList.FindEntityByClassname(NULL, "trigger_physics_trap");
	if (pRemover)
		return false;
	return Q_strcmp(pMapName, "d1_trainstation_03")		&& Q_strcmp(pMapName, "d1_trainstation_04")			&& Q_strcmp(pMapName, "d1_trainstation_05")
		&& Q_strcmp(pMapName, "d1_canals_02")			&& Q_strcmp(pMapName, "d1_canals_03")
		&& Q_strcmp(pMapName, "d1_eli_01")
		&& Q_strcmp(pMapName, "d1_town_04")
		&& (Q_strnicmp("d2_p", pMapName, 4) || !Q_strcmp(pMapName, "d2_prison_01"))//don't allow any prison map except 01 since that's outdoors
		&& Q_strcmp(pMapName, "d3_c17_01")				&& Q_strcmp(pMapName, "d3_c17_05")					&& Q_strcmp(pMapName, "d3_c17_06a")				&& Q_strcmp(pMapName, "d3_c17_06b")			&& Q_strcmp(pMapName, "d3_c17_10b")
		&& Q_strcmp(pMapName, "d3_citadel_02")			&& Q_strcmp(pMapName, "d3_citadel_05")
		&& Q_strcmp(pMapName, "d3_breen_01")

		&& Q_strcmp(pMapName, "ep1_citadel_02b")		&& Q_strcmp(pMapName, "ep1_citadel_03")
		&& Q_strcmp(pMapName, "ep1_c17_00")				&& Q_strcmp(pMapName, "ep1_c17_00a")				&& Q_strcmp(pMapName, "ep1_c17_02a")

		&& Q_strcmp(pMapName, "ep2_outland_01a")		&& Q_strcmp(pMapName, "ep2_outland_02")				&& Q_strcmp(pMapName, "ep2_outland_03")			&& Q_strcmp(pMapName, "ep2_outland_04")
		&& Q_strcmp(pMapName, "ep2_outland_11")			&& Q_strcmp(pMapName, "ep2_outland_11a")			&& Q_strcmp(pMapName, "ep2_outland_11b")		&& Q_strcmp(pMapName, "ep2_outland_12a");
}

CAI_BaseNPC *CChaosEffect::ChaosSpawnNPC(const char *className, string_t strActualName, int iSpawnType, const char *strModel, const char *strTargetname, const char *strWeapon, bool bEvil)
{
	float flDistAway;
	float flExtraHeight;
	switch (iSpawnType)
	{
	case SPAWNTYPE_EYELEVEL_REGULAR:
		flDistAway = 128;
		flExtraHeight = 64;
		break;
	case SPAWNTYPE_EYELEVEL_SPECIAL:
	case SPAWNTYPE_CEILING:
	case SPAWNTYPE_UNDERGROUND:
		flDistAway = 128;
		flExtraHeight = 64;
		break;
	case SPAWNTYPE_BIGFLYER:
		flDistAway = 192;
		flExtraHeight = 256;
		break;
	case SPAWNTYPE_STRIDER:
		flDistAway = 128;
		flExtraHeight = 512;
		break;
	default:
		return NULL;
	}
	char modDir[MAX_PATH];
	if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
		return NULL;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	//CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();//This will always give back some actual XY numbers because the camera is actually limited to 89 degrees up or down, not 90
	//then again something weird COULD happen to set the angle to straight 90 up/down, but idk what that would be
	CAI_BaseNPC *pNPC = (CAI_BaseNPC *)CreateEntityByName(className);
	if (pNPC)
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * flDistAway + Vector(0, 0, flExtraHeight);
		if (iSpawnType == SPAWNTYPE_CEILING)//put the NPC on the ceiling
		{
			trace_t tr;
			UTIL_TraceLine(vecOrigin, vecOrigin + Vector(0, 0, 100000), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);
			vecOrigin = tr.endpos + Vector(0, 0, -2);//move down a bit so barnacle looks right
		}
		float flPitch = 0;
		QAngle aAngles;
		if (iSpawnType == SPAWNTYPE_UNDERGROUND)//put the NPC in the ground
		{
			trace_t tr;
			UTIL_TraceLine(vecOrigin, vecOrigin - Vector(0, 0, 100000), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);
			vecOrigin = tr.endpos + Vector(0, 0, 1);//a little above the ground to avoid z fighting
			Vector xaxis(1.0f, 0.0f, 0.0f);
			pPlayer->EyeVectors(&xaxis);
			xaxis = -xaxis;
			Vector yaxis;
			CrossProduct(tr.plane.normal, xaxis, yaxis);
			if (VectorNormalize(yaxis) < 1e-3)
			{
				xaxis.Init(0.0f, 0.0f, 1.0f);
				CrossProduct(tr.plane.normal, xaxis, yaxis);
				VectorNormalize(yaxis);
			}
			CrossProduct(yaxis, tr.plane.normal, xaxis);
			VectorNormalize(xaxis);
			VMatrix entToWorld;
			entToWorld.SetBasisVectors(xaxis, yaxis, tr.plane.normal);
			MatrixToAngles(entToWorld, aAngles);
		}
		else
		{
			aAngles = QAngle(flPitch, pPlayer->GetAbsAngles().y - 180, 0);
		}
		pNPC->SetAbsOrigin(vecOrigin);
		pNPC->SetAbsAngles(aAngles);
		pNPC->m_bEvil = bEvil;
		if (FStrEq(className, "npc_alyx")) pNPC->KeyValue("ShouldHaveEMP", "1");
		if (FStrEq(className, "npc_cscanner")) pNPC->KeyValue("ShouldInspect", "1");
		if (FStrEq(className, "npc_sniper")) pNPC->AddSpawnFlags(65536);
		if (FStrEq(className, "npc_strider")) pNPC->AddSpawnFlags(65536);
		if (FStrEq(className, "npc_vortigaunt")) pNPC->KeyValue("ArmorRechargeEnabled", "1");
		if (FStrEq(className, "npc_apcdriver"))
		{
			pNPC->KeyValue("vehicle", "apc");
			pNPC->KeyValue("parentname", "apc");//once i saw an APC do nothing when it should have. maybe the driver was stuck in the wall and couldn't see, while the APC was teleported with GetUnstuck?
		}
		if (FStrEq(className, "npc_antlion"))
		{
			if (!Q_strcmp(modDir, "ep2chaos"))
			{
				if (random->RandomInt(0, 1) == 1)//cave variant
					pNPC->AddSpawnFlags(262144);
			}
			pNPC->KeyValue("startburrowed", "0");
		}
		if (FStrEq(className, "npc_antlionguard"))
		{
			if (!Q_strcmp(modDir, "ep2chaos"))
			{
				if (random->RandomInt(0, 1) == 1)//cave variant
				{
					pNPC->KeyValue("cavernbreed", "1");
					pNPC->KeyValue("incavern", "1");
				}
			}
			pNPC->KeyValue("startburrowed", "0");
			pNPC->KeyValue("allowbark", "1");
			pNPC->KeyValue("shovetargets", "*");
		}
		if (FStrEq(className, "npc_citizen"))
		{
			pNPC->AddSpawnFlags(65536);//follow player
			pNPC->AddSpawnFlags(262144);//random head
			if (random->RandomInt(0, 1) == 1)//medic
				pNPC->AddSpawnFlags(131072);
			if (random->RandomInt(0, 1) == 1)//ammo resupplier
			{
				float nRandom = random->RandomInt(0, 10);
				if (nRandom == 0) pNPC->KeyValue("ammosupply", "AR2");
				if (nRandom == 1) pNPC->KeyValue("ammosupply", "Pistol");
				if (nRandom == 2) pNPC->KeyValue("ammosupply", "SMG1");
				if (nRandom == 3) pNPC->KeyValue("ammosupply", "357");
				if (nRandom == 4) pNPC->KeyValue("ammosupply", "XBowBolt");
				if (nRandom == 5) pNPC->KeyValue("ammosupply", "Buckshot");
				if (nRandom == 6) pNPC->KeyValue("ammosupply", "RPG_Round");
				if (nRandom == 7) pNPC->KeyValue("ammosupply", "SMG1_Grenade");
				if (nRandom == 8) pNPC->KeyValue("ammosupply", "Grenade");
				if (nRandom == 9) pNPC->KeyValue("ammosupply", "Battery");
				if (nRandom == 10) pNPC->KeyValue("ammosupply", "AR2AltFire");
				pNPC->AddSpawnFlags(524288);
				pNPC->KeyValue("ammoamount", "100");
			}
			int nRandom = random->RandomInt(0, 6);//weapon
			if (nRandom == 0) pNPC->KeyValue("additionalequipment", "weapon_ar2");
			if (nRandom == 1) pNPC->KeyValue("additionalequipment", "weapon_citizenpackage");
			if (nRandom == 2) pNPC->KeyValue("additionalequipment", "weapon_citizensuitcase");
			if (nRandom == 3) pNPC->KeyValue("additionalequipment", "weapon_crowbar");
			if (nRandom == 4) pNPC->KeyValue("additionalequipment", "weapon_rpg");
			if (nRandom == 5) pNPC->KeyValue("additionalequipment", "weapon_shotgun");
			if (nRandom == 6) pNPC->KeyValue("additionalequipment", "weapon_smg1");

			nRandom = random->RandomInt(0, 3);//clothing
			if (nRandom == 0) pNPC->KeyValue("citizentype", "0");
			if (nRandom == 1) pNPC->KeyValue("citizentype", "1");
			if (nRandom == 2) pNPC->KeyValue("citizentype", "2");
			if (nRandom == 3) pNPC->KeyValue("citizentype", "3");

			pNPC->KeyValue("expressiontype", "0");
		}
		if (FStrEq(className, "npc_combine_s"))
		{
			pNPC->KeyValue("NumGrenades", "100");
			int nRandom = random->RandomInt(0, 2);//model/elite status
			if (nRandom == 0) pNPC->KeyValue("model", "models/combine_soldier.mdl");
			if (nRandom == 1) pNPC->KeyValue("model", "models/combine_super_soldier.mdl");
			if (nRandom == 2) pNPC->KeyValue("model", "models/combine_soldier_prisonguard.mdl");

			nRandom = random->RandomInt(0, 2);//weapon
			if (nRandom == 0) pNPC->KeyValue("additionalequipment", "weapon_ar2");
			if (nRandom == 1) pNPC->KeyValue("additionalequipment", "weapon_shotgun");
			if (nRandom == 2) pNPC->KeyValue("additionalequipment", "weapon_smg1");
		}
		if (FStrEq(className, "npc_combinedropship"))
		{
			int nRandom = 1;// random->RandomInt(-3, 1);//cargo type. avoid 0 cause that does nothing
			Msg("crate type %i\n", nRandom);
			if (nRandom == -3) pNPC->KeyValue("CrateType", "-3");//jeep
			if (nRandom == -2)//apc
			{
				pNPC->KeyValue("CrateType", "-2");
				ChaosSpawnVehicle("prop_vehicle_apc", strActualName, SPAWNTYPE_VEHICLE, "models/combine_apc.mdl", "chaos_dropship_apc", "scripts/vehicles/apc_npc.txt");
				pNPC->KeyValue("APCVehicleName", "chaos_dropship_apc");
				variant_t emptyVariant;
				g_EventQueue.AddEvent("combinedropship", "DropAPC", emptyVariant, 1, pNPC, pNPC);
			}
			if (nRandom == -1)//strider
			{
				pNPC->KeyValue("CrateType", "-1");
				variant_t emptyVariant;
				g_EventQueue.AddEvent("combinedropship", "DropStrider", emptyVariant, 1, pNPC, pNPC);
			}
			if (nRandom == 0) pNPC->KeyValue("CrateType", "2");//nothing
			if (nRandom == 1)//soldier crate
			{
				pNPC->KeyValue("CrateType", "1");
				pNPC->KeyValue("GunRange", "2000");
				//soldiers are spawned in CNPC_CombineDropship::SpawnTroop
				CBaseEntity *pTarget = CreateEntityByName("info_target");
				pTarget->KeyValue("targetname", "dropship_target");
				pTarget->SetAbsOrigin(vecOrigin);
				pTarget->SetAbsAngles(aAngles);
				DispatchSpawn(pTarget);
				pNPC->KeyValue("LandTarget", "dropship_target");
				variant_t variant;
				variant.SetInt(6);
				if (random->RandomInt(0, 1))
					g_EventQueue.AddEvent("combinedropship", "LandLeaveCrate", variant, 1, pNPC, pNPC);
				else
					g_EventQueue.AddEvent("combinedropship", "LandTakeCrate", variant, 1, pNPC, pNPC);
			}
		}
		if (FStrEq(className, "npc_metropolice"))
		{
			pNPC->KeyValue("manhacks", "100");

			int nRandom = random->RandomInt(0, 2);//weapon
			if (nRandom == 0) pNPC->KeyValue("additionalequipment", "weapon_smg1");
			if (nRandom == 1) pNPC->KeyValue("additionalequipment", "weapon_pistol");
			if (nRandom == 2) pNPC->KeyValue("additionalequipment", "weapon_stunstick");
		}
		if (FStrEq(className, "npc_stalker"))
		{
			int nRandom = random->RandomInt(0, 2);
			if (nRandom == 0) pNPC->KeyValue("BeamPower", "0");
			if (nRandom == 1) pNPC->KeyValue("BeamPower", "1");
			if (nRandom == 2) pNPC->KeyValue("BeamPower", "2");
		}
		if (FStrEq(className, "npc_turret_ceiling"))
		{
			pNPC->SetMaxHealth(700);
			pNPC->SetHealth(700);
			pNPC->AddSpawnFlags(32);
		}
		if (FStrEq(className, "npc_turret_ground"))
		{
			CBaseEntity *pMover = CreateEntityByName("func_movelinear");
			pMover->SetAbsOrigin(vecOrigin);

			//get a vector for what's "upward" relative to the surface we're on
			Vector forward, right, up;
			AngleVectors(aAngles, &forward, &right, &up);
			//entity wants a QAngle though... so put the direction back to an angle. dumb!
			QAngle angUp;
			VectorAngles(up, angUp);

			char buf[512];
			Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angUp.x, angUp.y, angUp.z);
			pMover->KeyValue("movedir", buf);
			pMover->KeyValue("movedistance", "32");
			pMover->KeyValue("speed", "40");
			pMover->KeyValue("startposition", "0");
			pMover->KeyValue("startsound", "Streetwar.d3_c17_10b_doormove3");
			pMover->KeyValue("stopsound", "Streetwar.d3_c17_10b_metal_stop1");
			pMover->KeyValue("targetname", "turret_lift");
			pMover->KeyValue("OnFullyOpen", "turret_ground,Enable,,0,-1");
			g_iChaosSpawnCount++; pMover->KeyValue("chaosid", g_iChaosSpawnCount);
			pNPC->SetAbsOrigin(vecOrigin - (up * 8));//put turret below top part of the cover
			pNPC->SetAbsAngles(aAngles);
			pMover->m_bChaosPersist = true;
			pMover->m_bChaosSpawned = true;
			DispatchSpawn(pMover);
			pMover->Activate();
			CBaseEntity *pProp = CreateEntityByName("prop_dynamic_override");
			pProp->SetAbsOrigin(vecOrigin);
			pProp->SetAbsAngles(aAngles);
			pProp->KeyValue("model", "models/props_c17/turretcover.mdl");
			pProp->KeyValue("disableshadows", "1");
			pProp->KeyValue("solid", "6");
			g_iChaosSpawnCount++; pProp->KeyValue("chaosid", g_iChaosSpawnCount);
			pProp->m_bChaosPersist = true;
			pProp->m_bChaosSpawned = true;
			DispatchSpawn(pProp);
			pProp->Activate();
			pNPC->SetParent(pMover);
			pProp->SetParent(pMover);
			variant_t sVariant;
			pMover->AcceptInput("Open", pMover, pMover, sVariant, 0);
			CBaseEntity *pSound1 = CreateEntityByName("ambient_generic");
			pSound1->SetAbsOrigin(vecOrigin);
			pSound1->KeyValue("targetname", "turret_detected_sound");
			pSound1->KeyValue("message", "Streetwar.d3_c17_10b_alarm1");
			pSound1->KeyValue("spawnflags", "16");
			g_iChaosSpawnCount++; pSound1->KeyValue("chaosid", g_iChaosSpawnCount);
			pSound1->m_bChaosPersist = true;
			pSound1->m_bChaosSpawned = true;
			DispatchSpawn(pSound1);
			pSound1->Activate();
			g_EventQueue.AddEvent("turret_detected_sound", "PlaySound", sVariant, 0.01, pPlayer, pPlayer);
			g_EventQueue.AddEvent("turret_detected_sound", "StopSound", sVariant, 1.5, pPlayer, pPlayer);
			pNPC->KeyValue("OnDeath", "turret_lift,Close,,0,-1");
		}

		if (!FStrEq(strModel, "_"))
			pNPC->KeyValue("model", strModel);
		if (!FStrEq(strWeapon, "_"))
			pNPC->KeyValue("additionalequipment", strWeapon);
		pNPC->KeyValue("targetname", strTargetname);
		pNPC->m_bChaosSpawned = true;
		pNPC->m_bChaosPersist = true;
		g_iChaosSpawnCount++; pNPC->CBaseEntity::KeyValue("chaosid", g_iChaosSpawnCount);
		DispatchSpawn(pNPC);
		pNPC->Activate();
		if (iSpawnType != SPAWNTYPE_UNDERGROUND)//taken care of
			pNPC->Teleport(&vecOrigin, &aAngles, NULL);
		m_strHudName = strActualName;

		if (iSpawnType != SPAWNTYPE_UNDERGROUND)//put the NPC in the ground
			pNPC->GetUnstuck(500);
	}
	return pNPC;
}
void CEPullToPlayer::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		if (gEntList.FindEntityByName(NULL, "chaos_pull_to_player") == NULL)
		{
			CPhysExplosion *pExplo = (CPhysExplosion*)CBaseEntity::Create("env_physexplosion", pPlayer->GetAbsOrigin() + Vector(0, 0, 32), QAngle(0, 0, 0), pPlayer);
			pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
			pExplo->m_radius = 600;
			pExplo->m_bInvert = true;
			pExplo->AddSpawnFlags(SF_PHYSEXPLOSION_NODAMAGE);
			pExplo->SetParent(pPlayer);
			pExplo->SetName(MAKE_STRING("chaos_pull_to_player"));
			pExplo->m_bConstant = true;
			variant_t emptyVariant;
			g_EventQueue.AddEvent("chaos_pull_to_player", "Explode", emptyVariant, 0, pPlayer, pPlayer, 0);
		}
	}
}
void CEPullToPlayer::StopEffect()
{
	CBaseEntity *pEnt = gEntList.FindEntityByName(NULL, "chaos_pull_to_player");
	while (pEnt)
	{
		UTIL_Remove(pEnt);
		//there is another
		pEnt = gEntList.FindEntityByName(pEnt, "chaos_pull_to_player");
	}
}
void CEPullToPlayer::TransitionEffect()
{
	StartEffect();
}
bool CEPullToPlayer::CheckStrike(const CTakeDamageInfo &info)
{
	return (info.GetDamageType() & DMG_CRUSH | DMG_SLASH | DMG_BLAST) != 0;
}
void CEPushFromPlayer::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		if (gEntList.FindEntityByName(NULL, "chaos_push_from_player") == NULL)
		{
			CPhysExplosion *pExplo = (CPhysExplosion*)CBaseEntity::Create("env_physexplosion", pPlayer->GetAbsOrigin() + Vector(0, 0, 32), QAngle(0, 0, 0), pPlayer);
			pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
			pExplo->m_radius = 500;
			pExplo->m_bInvert = false;
			pExplo->AddSpawnFlags(SF_PHYSEXPLOSION_NODAMAGE);
			pExplo->SetParent(pPlayer);
			pExplo->SetName(MAKE_STRING("chaos_push_from_player"));
			pExplo->m_bConstant = true;
			variant_t emptyVariant;
			g_EventQueue.AddEvent("chaos_push_from_player", "Explode", emptyVariant, 0, pPlayer, pPlayer, 0);
		}
	}
}
void CEPushFromPlayer::StopEffect()
{
	CBaseEntity *pEnt = gEntList.FindEntityByName(NULL, "chaos_push_from_player");
	while (pEnt)
	{
		UTIL_Remove(pEnt);
		//there is another
		pEnt = gEntList.FindEntityByName(pEnt, "chaos_push_from_player");
	}
}
void CEPushFromPlayer::TransitionEffect()
{
	StartEffect();
}
void CERandomWeaponGive::StartEffect()
{
	UTIL_GetLocalPlayer()->EquipSuit();
	int nRandom;
	//TODO: harpoon, stunstick, slam, alyxgun, annabelle, citizenpackage, citizensuitcase, cubemap
	for (int iWeaponAttempts = 0; iWeaponAttempts <= 30; iWeaponAttempts++)
	{
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 10) : chaos_rng1.GetInt();
		if (nRandom == 0) if (ChaosSpawnWeapon("weapon_crowbar", MAKE_STRING("Give Crowbar"))) return;
		if (nRandom == 1) if (ChaosSpawnWeapon("weapon_physcannon", MAKE_STRING("Give Gravity Gun"))) return;
		if (nRandom == 2) if (ChaosSpawnWeapon("weapon_pistol", MAKE_STRING("Give Pistol"), 255, "Pistol")) return;
		if (nRandom == 3) if (ChaosSpawnWeapon("weapon_357", MAKE_STRING("Give .357 Magnum"), 32, "357")) return;
		if (nRandom == 4) if (ChaosSpawnWeapon("weapon_smg1", MAKE_STRING("Give SMG"), 255, "SMG1", 3, "smg1_grenade")) return;
		if (nRandom == 5) if (ChaosSpawnWeapon("weapon_ar2", MAKE_STRING("Give AR2"), 255, "AR2", 5, "AR2AltFire")) return;
		if (nRandom == 6) if (ChaosSpawnWeapon("weapon_shotgun", MAKE_STRING("Give Shotgun"), 255, "Buckshot")) return;
		if (nRandom == 7) if (ChaosSpawnWeapon("weapon_crossbow", MAKE_STRING("Give Crossbow"), 16, "XBowBolt")) return;
		if (nRandom == 8) if (ChaosSpawnWeapon("weapon_frag", MAKE_STRING("Give Grenade"), 5, "grenade")) return;
		if (nRandom == 9) if (ChaosSpawnWeapon("weapon_rpg", MAKE_STRING("Give RPG"), 3, "rpg_round")) return;
		if (nRandom == 10)
		{
			if (ChaosSpawnWeapon("weapon_bugbait", MAKE_STRING("Give Bugbait")))
			{
				GlobalEntity_Add(MAKE_STRING("antlion_allied"), gpGlobals->mapname, GLOBAL_ON);//antlions become friendly
				return;
			}
		}
	}
}
void CERandomVehicle::StartEffect()
{
	variant_t sVariant;
	int nRandom;
	char modDir[MAX_PATH];
	if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
		return;
	if (!Q_strcmp(modDir, "ep2chaos"))
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 5) : chaos_rng1.GetInt();
	else
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 4) : chaos_rng1.GetInt();
	//TODO: If there are two jalopies on the map, the radar stops working for them both? speculative fix
	if (nRandom == 5)
	{
		if (gEntList.FindEntityByClassname(NULL, "prop_vehicle_jeep"))//avoid radar issues that come up when there is more than one jalopy in the map at a time
		{
			nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 4) : chaos_rng1.GetInt();
		}
		else
		{
			CBaseEntity *pJalopy = ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("Spawn Jalopy"), SPAWNTYPE_VEHICLE, "models/vehicle.mdl", "jalopy", "scripts/vehicles/jalopy.txt");
			//give all the bells and whistles except compass thingy. don't know how that would react to multiple cars
			pJalopy->AcceptInput("EnableRadar", pJalopy, pJalopy, sVariant, 0);
			pJalopy->AcceptInput("EnableRadarDetectEnemies", pJalopy, pJalopy, sVariant, 0);
			pJalopy->AcceptInput("AddBusterToCargo", pJalopy, pJalopy, sVariant, 0);
		}
	}
	if (nRandom == 4)
	{
		CBaseEntity *pRemover = gEntList.FindEntityByClassname(NULL, "trigger_physics_trap");//avoid dissolver triggers, easy crash
		if (pRemover)
		{
			nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 3) : chaos_rng1.GetInt();
		}
		else
		{
			//crane
			CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
			CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(pPlayer);
			g_iChaosSpawnCount++;
			char szName[2048];

			//crane magnet
			Vector vecOrigin = pHL2Player->RotatedOffset(Vector(1034, 164, 750), true);
			QAngle vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
			CBaseEntity *pMagnet = CreateEntityByName("phys_magnet");
			pMagnet->KeyValue("model", "models/props_wasteland/cranemagnet01a.mdl");
			pMagnet->KeyValue("massScale", "1000");
			Q_snprintf(szName, sizeof(szName), "crane_magnet_%i", g_iChaosSpawnCount);
			pMagnet->KeyValue("targetname", szName);
			pMagnet->KeyValue("overridescript", "damping,0.2,rotdamping,0.2,inertia,0.3");
			DispatchSpawn(pMagnet);
			pMagnet->Activate();
			pMagnet->Teleport(&vecOrigin, &vecAngles, NULL);
			pMagnet->GetUnstuck(500, UF_NO_NODE_TELEPORT);

			//crane
			vecOrigin = pHL2Player->RotatedOffset(Vector(400, 0, 64), true);
			vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
			CBaseEntity *pVehicle = CreateEntityByName("prop_vehicle_crane");
			pVehicle->KeyValue("model", "models/Cranes/crane_docks.mdl");
			pVehicle->KeyValue("solid", "6");
			pVehicle->KeyValue("magnetname", szName);
			Q_snprintf(szName, sizeof(szName), "crane%i", g_iChaosSpawnCount);
			pVehicle->KeyValue("targetname", szName);
			pVehicle->KeyValue("vehiclescript", "scripts/vehicles/crane.txt");
			pVehicle->KeyValue("PlayerOn", "chaos_ladder,Disable,,0,-1");
			pVehicle->KeyValue("PlayerOff", "chaos_ladder,Enable,,5,-1");
			pVehicle->Teleport(&vecOrigin, &vecAngles, NULL);

			//crane ladder
			vecOrigin = pHL2Player->RotatedOffset(Vector(524, 84, 90), true);
			vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
			CFuncLadder *pLadder = (CFuncLadder *)CreateEntityByName("func_useableladder");
			pLadder->KeyValue("parentname", szName);
			pLadder->SetEndPoints(pHL2Player->RotatedOffset(Vector(524, 84, 90), true), pHL2Player->RotatedOffset(Vector(524, 84, 4), true));
			pLadder->KeyValue("targetname", "chaos_ladder");
			pLadder->KeyValue("spawnflags", "1");
			Q_snprintf(szName, sizeof(szName), "crane%i,ForcePlayerIn,,0,-1", g_iChaosSpawnCount);
			pLadder->KeyValue("OnPlayerGotOnLadder", szName);
			DispatchSpawn(pLadder);
			pLadder->Activate();
			pLadder->Teleport(&vecOrigin, &vecAngles, NULL);

			//activate crane last so everything works correctly
			DispatchSpawn(pVehicle);
			pVehicle->Activate();
			m_strHudName = MAKE_STRING("Spawn Crane");
		}
	}
	if (nRandom == 0) ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("Spawn Buggy"), SPAWNTYPE_VEHICLE, "models/buggy.mdl", "jeep", "scripts/vehicles/jeep_test.txt");
	if (nRandom == 1) ChaosSpawnVehicle("prop_vehicle_airboat", MAKE_STRING("Spawn Airboat"), SPAWNTYPE_VEHICLE, "models/airboat.mdl", "airboat", "scripts/vehicles/airboat.txt");
	if (nRandom == 2) ChaosSpawnVehicle("prop_vehicle_prisoner_pod", MAKE_STRING("Spawn Pod"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/vehicles/prisoner_pod_inner.mdl", "pod", "scripts/vehicles/prisoner_pod.txt");
	if (nRandom == 3) ChaosSpawnVehicle("prop_vehicle_apc", MAKE_STRING("Spawn APC"), SPAWNTYPE_VEHICLE, "models/combine_apc.mdl", "apc", "scripts/vehicles/apc_npc.txt");
}
void CERandomNPC::StartEffect()
{
	//CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	variant_t emptyVariant;
	variant_t sVariant;
	int nRandom;
	//TODO: blob
	char modDir[MAX_PATH];
	if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
		return;
	if (!Q_strcmp(modDir, "ep2chaos"))
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 46) : chaos_rng1.GetInt();
	else if (!Q_strcmp(modDir, "ep1chaos"))
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 41) : chaos_rng1.GetInt();
	else
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 40) : chaos_rng1.GetInt();
	if (nRandom == 0)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_alyx", MAKE_STRING("Spawn Alyx"), SPAWNTYPE_EYELEVEL_REGULAR, "models/alyx.mdl", "alyx", "weapon_alyxgun")->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 1) m_iSavedChaosID = ChaosSpawnNPC("npc_antlion", MAKE_STRING("Spawn Antlion"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "antlion", "_")->m_iChaosID;
	if (nRandom == 2) m_iSavedChaosID = ChaosSpawnNPC("npc_antlionguard", MAKE_STRING("Spawn Antlion Guard"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "antlionguard", "_")->m_iChaosID;
	if (nRandom == 3) m_iSavedChaosID = ChaosSpawnNPC("npc_barnacle", MAKE_STRING("Spawn Barnacle"), SPAWNTYPE_CEILING, "_", "barnacle", "_")->m_iChaosID;
	if (nRandom == 4)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_barney", MAKE_STRING("Spawn Barney"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "barney", "weapon_ar2")->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 5) m_iSavedChaosID = ChaosSpawnNPC("npc_breen", MAKE_STRING("Spawn Breen"), SPAWNTYPE_EYELEVEL_REGULAR, "models/breen.mdl", "breen", "_")->m_iChaosID;
	if (nRandom == 6)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_citizen", MAKE_STRING("Spawn Citizen"), SPAWNTYPE_EYELEVEL_REGULAR, "models/Humans/Group02/Male_05.mdl", "citizen", "_")->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 7) m_iSavedChaosID = ChaosSpawnNPC("npc_combine_s", MAKE_STRING("Spawn Combine Soldier"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "combine_s", "_")->m_iChaosID;
	if (nRandom == 8) m_iSavedChaosID = ChaosSpawnNPC("npc_combinedropship", MAKE_STRING("Spawn Dropship"), SPAWNTYPE_BIGFLYER, "_", "combinedropship", "_")->m_iChaosID;
	if (nRandom == 9) m_iSavedChaosID = ChaosSpawnNPC("npc_combinegunship", MAKE_STRING("Spawn Gunship"), SPAWNTYPE_BIGFLYER, "_", "combinegunship", "_")->m_iChaosID;
	if (nRandom == 10) m_iSavedChaosID = ChaosSpawnNPC("npc_crow", MAKE_STRING("Spawn Crow"), SPAWNTYPE_EYELEVEL_REGULAR, "models/crow.mdl", "crow", "_")->m_iChaosID;
	if (nRandom == 11) m_iSavedChaosID = ChaosSpawnNPC("npc_cscanner", MAKE_STRING("Spawn Scanner"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/combine_scanner.mdl", "cscanner", "_")->m_iChaosID;
	if (nRandom == 12)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_dog", MAKE_STRING("Spawn Dog"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "dog", "_")->m_iChaosID;
		GetEntityWithID(m_iSavedChaosID)->AcceptInput("StartCatchThrowBehavior", GetEntityWithID(m_iSavedChaosID), GetEntityWithID(m_iSavedChaosID), emptyVariant, 0);
	}
	if (nRandom == 13) m_iSavedChaosID = ChaosSpawnNPC("npc_eli", MAKE_STRING("Spawn Eli"), SPAWNTYPE_EYELEVEL_REGULAR, "models/eli.mdl", "eli", "_")->m_iChaosID;
	if (nRandom == 14) m_iSavedChaosID = ChaosSpawnNPC("npc_fastzombie", MAKE_STRING("Spawn Fast Zombie"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "fastzombie", "_")->m_iChaosID;
	if (nRandom == 15) m_iSavedChaosID = ChaosSpawnNPC("npc_gman", MAKE_STRING("Spawn Gman"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "gman", "_")->m_iChaosID;
	if (nRandom == 16) m_iSavedChaosID = ChaosSpawnNPC("npc_headcrab", MAKE_STRING("Spawn Headcrab"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "headcrab", "_")->m_iChaosID;
	if (nRandom == 17) m_iSavedChaosID = ChaosSpawnNPC("npc_headcrab_black", MAKE_STRING("Spawn Poison Headcrab"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "headcrab_black", "_")->m_iChaosID;
	if (nRandom == 18) m_iSavedChaosID = ChaosSpawnNPC("npc_headcrab_fast", MAKE_STRING("Spawn Fast Headcrab"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "headcrab_fast", "_")->m_iChaosID;
	if (nRandom == 19) m_iSavedChaosID = ChaosSpawnNPC("npc_helicopter", MAKE_STRING("Spawn Hunter-Chopper"), SPAWNTYPE_BIGFLYER, "_", "helicopter", "_")->m_iChaosID;
	if (nRandom == 20) m_iSavedChaosID = ChaosSpawnNPC("npc_ichthyosaur", MAKE_STRING("Spawn Ichthyosaur"), SPAWNTYPE_EYELEVEL_SPECIAL, "_", "ichthyosaur", "_")->m_iChaosID;
	if (nRandom == 21) m_iSavedChaosID = ChaosSpawnNPC("npc_kleiner", MAKE_STRING("Spawn Dr. Kleiner"), SPAWNTYPE_EYELEVEL_REGULAR, "models/kleiner.mdl", "kleiner", "_")->m_iChaosID;
	if (nRandom == 22) m_iSavedChaosID = ChaosSpawnNPC("npc_manhack", MAKE_STRING("Spawn Manhack"), SPAWNTYPE_EYELEVEL_SPECIAL, "_", "manhack", "_")->m_iChaosID;
	if (nRandom == 23) m_iSavedChaosID = ChaosSpawnNPC("npc_metropolice", MAKE_STRING("Spawn CP Unit"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "metropolice", "_")->m_iChaosID;
	if (nRandom == 24) m_iSavedChaosID = ChaosSpawnNPC("npc_monk", MAKE_STRING("Spawn Father Grigori"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "monk", "weapon_annabelle")->m_iChaosID;
	if (nRandom == 25) m_iSavedChaosID = ChaosSpawnNPC("npc_mossman", MAKE_STRING("Spawn Dr. Mossman"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "mossman", "_")->m_iChaosID;
	if (nRandom == 26) m_iSavedChaosID = ChaosSpawnNPC("npc_pigeon", MAKE_STRING("Spawn Pigeon"), SPAWNTYPE_EYELEVEL_REGULAR, "models/pigeon.mdl", "pigeon", "_")->m_iChaosID;
	if (nRandom == 27) m_iSavedChaosID = ChaosSpawnNPC("npc_poisonzombie", MAKE_STRING("Spawn Poison Zombie"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "poisonzombie", "_")->m_iChaosID;
	if (nRandom == 28) m_iSavedChaosID = ChaosSpawnNPC("npc_rollermine", MAKE_STRING("Spawn Rollermine"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "rollermine", "_")->m_iChaosID;
	if (nRandom == 29) m_iSavedChaosID = ChaosSpawnNPC("npc_seagull", MAKE_STRING("Spawn Seagull"), SPAWNTYPE_EYELEVEL_REGULAR, "models/seagull.mdl", "seagull", "_")->m_iChaosID;
	if (nRandom == 30) m_iSavedChaosID = ChaosSpawnNPC("npc_sniper", MAKE_STRING("Spawn Sniper"), SPAWNTYPE_EYELEVEL_SPECIAL, "_", "sniper", "_")->m_iChaosID;
	if (nRandom == 31) m_iSavedChaosID = ChaosSpawnNPC("npc_stalker", MAKE_STRING("Spawn Stalker"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "stalker", "_")->m_iChaosID;
	if (nRandom == 32) m_iSavedChaosID = ChaosSpawnNPC("npc_strider", MAKE_STRING("Spawn Strider"), SPAWNTYPE_STRIDER, "models/combine_strider.mdl", "strider", "_")->m_iChaosID;
	if (nRandom == 33) m_iSavedChaosID = ChaosSpawnNPC("npc_turret_ceiling", MAKE_STRING("Spawn Ceiling Turret"), SPAWNTYPE_CEILING, "_", "turret_ceiling", "_")->m_iChaosID;
	if (nRandom == 34) m_iSavedChaosID = ChaosSpawnNPC("npc_turret_floor", MAKE_STRING("Spawn Turret"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "turret_floor", "_")->m_iChaosID;
	if (nRandom == 35) m_iSavedChaosID = ChaosSpawnNPC("npc_turret_ground", MAKE_STRING("Spawn Ground Turret"), SPAWNTYPE_UNDERGROUND, "_", "turret_ground", "_")->m_iChaosID;
	if (nRandom == 36)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_vortigaunt", MAKE_STRING("Spawn Vortigaunt"), SPAWNTYPE_EYELEVEL_REGULAR, "models/vortigaunt.mdl", "vortigaunt", "_")->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 37) m_iSavedChaosID = ChaosSpawnNPC("npc_zombie", MAKE_STRING("Spawn Zombie"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "zombie", "_")->m_iChaosID;
	if (nRandom == 38) m_iSavedChaosID = ChaosSpawnNPC("npc_zombie_torso", MAKE_STRING("Spawn Zombie Torso"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "zombie_torso", "_")->m_iChaosID;
	if (nRandom == 39)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_fisherman", MAKE_STRING("Spawn Fisherman"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "fisherman", "weapon_oldmanharpoon")->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 40)
	{
		ChaosSpawnVehicle("prop_vehicle_apc", MAKE_STRING("Spawn APC (!)"), SPAWNTYPE_VEHICLE, "models/combine_apc.mdl", "apc", "scripts/vehicles/apc_npc.txt");
		m_iSavedChaosID = ChaosSpawnNPC("npc_apcdriver", MAKE_STRING("Spawn APC (!)"), SPAWNTYPE_EYELEVEL_SPECIAL, "_", "apcdriver", "_")->m_iChaosID;
		//make apc follow player
		/*
		sVariant.SetString(MAKE_STRING("!player"));
		g_EventQueue.AddEvent("apcdriver", "GotoPathCorner", sVariant, 0, pPlayer, pPlayer, 0);
		g_EventQueue.AddEvent("apcdriver", "GotoPathCorner", sVariant, 10, pPlayer, pPlayer, 0);
		g_EventQueue.AddEvent("apcdriver", "GotoPathCorner", sVariant, 20, pPlayer, pPlayer, 0);
		g_EventQueue.AddEvent("apcdriver", "GotoPathCorner", sVariant, 30, pPlayer, pPlayer, 0);
		g_EventQueue.AddEvent("apcdriver", "GotoPathCorner", sVariant, 40, pPlayer, pPlayer, 0);
		g_EventQueue.AddEvent("apcdriver", "GotoPathCorner", sVariant, 50, pPlayer, pPlayer, 0);
		g_EventQueue.AddEvent("apcdriver", "GotoPathCorner", sVariant, 60, pPlayer, pPlayer, 0);
		*/
	}
	//ep1
	if (nRandom == 41) m_iSavedChaosID = ChaosSpawnNPC("npc_zombine", MAKE_STRING("Spawn Zombine"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "zombine", "_")->m_iChaosID;
	//ep2
	if (nRandom == 42) m_iSavedChaosID = ChaosSpawnNPC("npc_advisor", MAKE_STRING("Spawn Advisor"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/advisor.mdl", "advisor", "_")->m_iChaosID;
	if (nRandom == 43) m_iSavedChaosID = ChaosSpawnNPC("npc_antlion_grub", MAKE_STRING("Spawn Antlion Grub"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "antlion_grub", "_")->m_iChaosID;
	if (nRandom == 44) m_iSavedChaosID = ChaosSpawnNPC("npc_fastzombie_torso", MAKE_STRING("Spawn Fast Zombie Torso"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "fastzombie_torso", "_")->m_iChaosID;
	if (nRandom == 45) m_iSavedChaosID = ChaosSpawnNPC("npc_hunter", MAKE_STRING("Spawn Hunter"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "hunter", "_")->m_iChaosID;
	if (nRandom == 46) m_iSavedChaosID = ChaosSpawnNPC("npc_magnusson", MAKE_STRING("Spawn Dr. Magnusson"), SPAWNTYPE_EYELEVEL_REGULAR, "models/magnusson.mdl", "magnusson", "_")->m_iChaosID;
}
bool CERandomNPC::CheckStrike(const CTakeDamageInfo &info)
{
	return info.GetAttacker() == GetEntityWithID(m_iSavedChaosID);
}
void CERandomNPC::MaintainEffect()
{
	CBaseEntity *pNPC = GetEntityWithID(m_iSavedChaosID);
	if (!pNPC)//if NPC died
		return;
}
void CELockVehicles::DoOnVehicles(CPropVehicleDriveable *pVehicle)
{
	variant_t emptyVariant;
	if (m_flTimeRem < 1 || !m_bActive)
		pVehicle->AcceptInput("Unlock", pVehicle, pVehicle, emptyVariant, 0);
	else
		pVehicle->AcceptInput("Lock", pVehicle, pVehicle, emptyVariant, 0);
}
void CELockVehicles::StartEffect()
{
	IterUsableVehicles(false);
}
void CELockVehicles::StopEffect()
{
	IterUsableVehicles(false);
}
void CESuperMovement::StartEffect()
{
	sv_maxspeed.SetValue(4000);//320
	hl2_normspeed.SetValue(4000);//190
	hl2_sprintspeed.SetValue(4000);//320
	hl2_walkspeed.SetValue(4000);//150
	hl2_duckspeed.SetValue(4000);//64
	static ConVar *pCVcl_forwardspeed = (ConVar *)cvar->FindVar("cl_forwardspeed");
	pCVcl_forwardspeed->SetValue(4000);//450
	static ConVar *pCVcl_sidespeed = (ConVar *)cvar->FindVar("cl_sidespeed");
	pCVcl_sidespeed->SetValue(4000);//450
	static ConVar *pCVcl_upspeed = (ConVar *)cvar->FindVar("cl_upspeed");
	pCVcl_upspeed->SetValue(4000);//320
	static ConVar *pCVcl_backspeed = (ConVar *)cvar->FindVar("cl_backspeed");
	pCVcl_backspeed->SetValue(4000);//450
	UTIL_GetLocalPlayer()->SetMaxSpeed(HL2_NORM_SPEED);
}
void CESuperMovement::StopEffect()
{
	sv_maxspeed.SetValue(320);//
	hl2_normspeed.SetValue(190);//
	hl2_sprintspeed.SetValue(320);//
	hl2_walkspeed.SetValue(150);//
	hl2_duckspeed.SetValue(64);//
	static ConVar *pCVcl_forwardspeed = (ConVar *)cvar->FindVar("cl_forwardspeed");
	pCVcl_forwardspeed->SetValue(450);//
	static ConVar *pCVcl_sidespeed = (ConVar *)cvar->FindVar("cl_sidespeed");
	pCVcl_sidespeed->SetValue(450);//
	static ConVar *pCVcl_upspeed = (ConVar *)cvar->FindVar("cl_upspeed");
	pCVcl_upspeed->SetValue(320);//
	static ConVar *pCVcl_backspeed = (ConVar *)cvar->FindVar("cl_backspeed");
	pCVcl_backspeed->SetValue(450);//
	UTIL_GetLocalPlayer()->SetMaxSpeed(HL2_NORM_SPEED);
	if (g_ChaosEffects[EFFECT_NO_MOVEMENT]->m_bActive && g_ChaosEffects[EFFECT_NO_MOVEMENT]->m_flTimeRem > 1)
	{
		//EFFECT_NO_MOVEMENT is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_NO_MOVEMENT]->StartEffect();
	}
}
void CESolidTriggers::StartEffect()
{
	CBaseEntity *pEnt = gEntList.FirstEnt();
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	while (pEnt)
	{
		CBaseTrigger *pBaseTrigger = dynamic_cast<CBaseTrigger *>(pEnt);
		CTriggerVPhysicsMotion *pTriggerVPhysicsMotion = dynamic_cast<CTriggerVPhysicsMotion *>(pEnt);
		bool bBool = (pBaseTrigger != NULL) || (pTriggerVPhysicsMotion != NULL);
		if (bBool && ((pBaseTrigger != NULL) ? !pBaseTrigger->m_bDisabled : !pTriggerVPhysicsMotion->m_bDisabled))
		{
			pEnt->RemoveEffects(EF_NODRAW);
			if (pEnt->VPhysicsGetObject())
			{
				pEnt->VPhysicsGetObject()->EnableCollisions(true);
			}
			pEnt->RemoveSolidFlags(FSOLID_TRIGGER);
			pEnt->RemoveSolidFlags(FSOLID_NOT_SOLID);
			pEnt->PhysicsTouchTriggers();
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
	//if we're still stuck, then we're probably in some HUGE triggers. just make them unsolid again
	//500 is as far as i feel comfortable teleporting the player. HUGE triggers could make us teleport outside of entire buildings and such
	while (!pPlayer->GetUnstuck(500, UF_NO_NODE_TELEPORT))
	{
		trace_t	trace;
		UTIL_TraceEntity(pPlayer, pPlayer->GetAbsOrigin(), pPlayer->GetAbsOrigin(), MASK_PLAYERSOLID, &trace);
		trace.m_pEnt->AddEffects(EF_NODRAW);
		if (trace.m_pEnt->VPhysicsGetObject())
		{
			trace.m_pEnt->VPhysicsGetObject()->EnableCollisions(true);
		}
		CTriggerVPhysicsMotion *pTriggerVPhysicsMotion = dynamic_cast<CTriggerVPhysicsMotion *>(trace.m_pEnt);
		if (!pTriggerVPhysicsMotion)//trigger_vphysics_motion doesn't use this according to a comment in CBaseVPhysicsTrigger::Spawn()
			trace.m_pEnt->AddSolidFlags(FSOLID_TRIGGER);
		trace.m_pEnt->AddSolidFlags(FSOLID_NOT_SOLID);
		trace.m_pEnt->PhysicsTouchTriggers();
	}
}
void CESolidTriggers::StopEffect()
{
	g_bEndSolidTriggers = true;
	CBaseEntity *pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		CBaseTrigger *pBaseTrigger = dynamic_cast<CBaseTrigger *>(pEnt);
		CTriggerVPhysicsMotion *pTriggerVPhysicsMotion = dynamic_cast<CTriggerVPhysicsMotion *>(pEnt);
		bool bBool = (pBaseTrigger != NULL) || (pTriggerVPhysicsMotion != NULL);
		if (bBool && ((pBaseTrigger != NULL) ? !pBaseTrigger->m_bDisabled : !pTriggerVPhysicsMotion->m_bDisabled))
		{
			pEnt->AddEffects(EF_NODRAW);
			if (pEnt->VPhysicsGetObject())
			{
				pEnt->VPhysicsGetObject()->EnableCollisions(true);
			}
			if (!pTriggerVPhysicsMotion)//trigger_vphysics_motion doesn't use this according to a comment in CBaseVPhysicsTrigger::Spawn()
				pEnt->AddSolidFlags(FSOLID_TRIGGER);//TODO: this may also fix the crash related to the use of g_bEndSolidTriggers
			pEnt->AddSolidFlags(FSOLID_NOT_SOLID);
			pEnt->PhysicsTouchTriggers();
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
	g_bEndSolidTriggers = false;
}
void CESolidTriggers::TransitionEffect()
{
	StartEffect();
}
void CECredits::StartEffect()
{
	//hack effect length so we can play song 2
	m_flDuration = 100;
	m_flTimeRem = 100;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	//visual
	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();
	UserMessageBegin(user, "CreditsMsg");
	WRITE_BYTE(3);
	MessageEnd();
	//audio
	CPASAttenuationFilter filter(pPlayer);
	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	ep.m_pSoundName = "*#music/hl2_song3.mp3";
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NORM;
	ep.m_nPitch = PITCH_NORM;
	pPlayer->EmitSound(filter, pPlayer->entindex(), ep);
}
void CECredits::MaintainEffect()
{
	if (m_flTimeRem > 13.5 || m_bPlayedSecondSong)//song 2 delayed by ? seconds
		return;
	m_bPlayedSecondSong = true;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CPASAttenuationFilter filter(pPlayer);
	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	ep.m_pSoundName = "*#music/hl1_song25_remix3.mp3";
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NORM;
	ep.m_nPitch = PITCH_NORM;
	pPlayer->EmitSound(filter, pPlayer->entindex(), ep);
}
void CECredits::RestoreEffect()
{
	//if we reload, the credits visual disappears and as far as i know we can't put it back to where it was at time of load, so we don't bother restoring it
	//playing song 2 would make no sense without the visual
	m_bPlayedSecondSong = true;
}
void CECredits::TransitionEffect()
{
	//if we reload, the credits visual disappears and as far as i know we can't put it back to where it was at time of load, so we don't bother restoring it
	//playing song 2 would make no sense without the visual
	m_bPlayedSecondSong = true;
}
void CESuperhot::FastThink()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer->pl.deadflag)
	{
		cvar->FindVar("host_timescale")->SetValue(1);
		return;
	}
	CBaseEntity *pVehicle = pPlayer->GetVehicleEntity();
	Vector vecVelocity;
	if (pVehicle && pVehicle->VPhysicsGetObject())
	{
		pVehicle->VPhysicsGetObject()->GetVelocity(&vecVelocity, NULL);
	}
	else
	{
		vecVelocity = pPlayer->GetAbsVelocity();
	}
	//If input locks up, change 0.07 to something higher
	float flNum = min(3, 2 * max(0.07, 1 / (hl2_normspeed.GetFloat() / max(hl2_normspeed.GetFloat() * 0.05, vecVelocity.Length()))));
	cvar->FindVar("host_timescale")->SetValue(flNum);
}
void CESupercold::FastThink()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer->pl.deadflag)
	{
		cvar->FindVar("host_timescale")->SetValue(1);
		return;
	}
	CBaseEntity *pVehicle = pPlayer->GetVehicleEntity();
	Vector vecVelocity;
	if (pVehicle && pVehicle->VPhysicsGetObject())
	{
		pVehicle->VPhysicsGetObject()->GetVelocity(&vecVelocity, NULL);
	}
	else
	{
		vecVelocity = pPlayer->GetAbsVelocity();
	}
	float flNum = max(0.3, (hl2_normspeed.GetFloat() / max(hl2_normspeed.GetFloat() * 0.3, vecVelocity.Length())));
	cvar->FindVar("host_timescale")->SetValue(flNum);
}
void CEPullToPlayer::MaintainEffect()
{
	CBaseEntity *pEnt = gEntList.FindEntityByName(NULL, "chaos_pull_to_player");
	if (pEnt)
	{
		CPhysExplosion *pExplo = static_cast<CPhysExplosion*>(pEnt);
		pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
	}
	else
	{
		StartEffect();
	}
}
void CEPushFromPlayer::MaintainEffect()
{
	CBaseEntity *pEnt = gEntList.FindEntityByName(NULL, "chaos_push_from_player");
	if (pEnt)
	{
		CPhysExplosion *pExplo = static_cast<CPhysExplosion*>(pEnt);
		pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
	}
	else
	{
		StartEffect();
	}
}
void CEColors::StartEffect()
{
	CBaseEntity *pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		if (pEnt->ClassMatches("env_fo*"))
		{
			//change fog!
			variant_t colorVariant;
			colorVariant.SetColor32(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), pEnt->GetRenderColor().a);
			pEnt->AcceptInput("SetColor", pEnt, pEnt, colorVariant, 0);
			colorVariant.SetColor32(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), pEnt->GetRenderColor().a);
			pEnt->AcceptInput("SetColorSecondary", pEnt, pEnt, colorVariant, 0);
		}
		//doing it the input way allows us to hit beams and other things that have their own coloring process
		char szcolor[2048];
		variant_t colorVariant;
		int r = random->RandomInt(0, 255);
		int g = random->RandomInt(0, 255);
		int b = random->RandomInt(0, 255);
		Q_snprintf(szcolor, sizeof(szcolor), "%i %i %i", r, g, b);
		Msg("%s\n", szcolor);
		colorVariant.SetString(MAKE_STRING(szcolor));
		pEnt->AcceptInput("Color", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), colorVariant, 0);
		pEnt = gEntList.NextEnt(pEnt);
	}
}
void CEColors::MaintainEffect()
{
	CBaseEntity *pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		//make sure we don't recolor things that we already colored. this aint a rave.
		if (255 == pEnt->GetRenderColor().r == pEnt->GetRenderColor().g == pEnt->GetRenderColor().b)
		{
			//doing it the input way allows us to hit beams and other things that have their own coloring process
			char szcolor[2048];
			variant_t colorVariant;
			int r = random->RandomInt(0, 255);
			int g = random->RandomInt(0, 255);
			int b = random->RandomInt(0, 255);
			Q_snprintf(szcolor, sizeof(szcolor), "%i %i %i", r, g, b);
			Msg("%s\n", szcolor);
			colorVariant.SetString(MAKE_STRING(szcolor));
			pEnt->AcceptInput("Color", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), colorVariant, 0);
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
}

//is the given character one that the game will force you to keep alive
bool IsPlayerAlly(CBaseCombatCharacter *pCharacter)
{
	if (pCharacter->m_bChaosSpawned)
		return false;
		
	if (pCharacter->IsNPC())
	{
		if (pCharacter->GetMaxHealth() < 10)//talkers (max health of 8 for all of them apparently?)
			return true;
		if (pCharacter->MyNPCPointer()->IsPlayerAlly())//fighters
		{
			//damage filter pertains to some invulnerable citizens in some maps
			if (pCharacter->m_hDamageFilter)
				return true;
			if (pCharacter->ClassMatches("npc_a*") || pCharacter->ClassMatches("npc_b*") || pCharacter->ClassMatches("npc_v*"))
				return true;
		}
	}
	return false;
}
void CENPCRels::DoNPCRels(int disposition, bool bRevert)
{
	const int MAX_HANDLED = 512;
	CUtlVectorFixed<CBaseCombatCharacter *, MAX_HANDLED> subjectList;
	CUtlVectorFixed<CBaseCombatCharacter *, MAX_HANDLED> targetList;
	//Search players first
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer	*pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			subjectList.AddToTail(pPlayer);
			targetList.AddToTail(pPlayer);
		}
	}
	for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
	{
		CAI_BaseNPC *pNPC = (g_AI_Manager.AccessAIs())[i];
		if (pNPC)
		{
			subjectList.AddToTail(pNPC);
			targetList.AddToTail(pNPC);
		}
	}
	if (subjectList.Count() == 0)
	{
		Warning("Found no subjects for chaos effect!?\n");
		return;
	}
	else if (targetList.Count() == 0)
	{
		Warning("Found no targets for chaos effect!?\n");
		return;
	}
	for (int i = 0; i < subjectList.Count(); i++)
	{
		CBaseCombatCharacter *pSubject = subjectList[i];
		for (int j = 0; j < targetList.Count(); j++)
		{
			CBaseCombatCharacter *pTarget = targetList[j];
			if (pSubject == pTarget)
				continue;

			if (disposition == D_HT || disposition == D_FR)
			{
				//don't make vital allies like alyx and barney hostile to player
				//you can't just kill them!
				bool bTargetIsPlayer = pTarget->IsPlayer();
				bool bSubjectIsAlly = IsPlayerAlly(pSubject);
				if (bTargetIsPlayer && bSubjectIsAlly)
					continue;

				//check other way too so player can't kill them by friendly fire cause that's just dumb
				bool bSubjectIsPlayer = pSubject->IsPlayer();
				bool bTargetIsAlly = IsPlayerAlly(pTarget);
				if (bSubjectIsPlayer && bTargetIsAlly)
					continue;

				//no fighting amongst themselves either
				if (bSubjectIsAlly && bTargetIsAlly)
					continue;

				//don't try to attack a crane driver, you're not winning
				if (pTarget->ClassMatches("npc_cra*"))
					pSubject->AddEntityRelationship(pTarget, D_NU, 100);
			}

			if (bRevert)
			{
				pSubject->RemoveEntityRelationship(pTarget);
			}
			else
			{
				if (pSubject->IRelationType(pTarget) != disposition || pSubject->IRelationPriority(pTarget) != 100)
				{
					pSubject->AddEntityRelationship(pTarget, (Disposition_t)disposition, 100);
					//Msg("Applying relationship to %s and %s\n", STRING(pSubject->GetEntityName()), STRING(pTarget->GetEntityName()));
				}
			}
		}
	}
}
void CENPCRels::StopEffect()
{
	DoNPCRels(D_ER, true);
	if (m_nID == EFFECT_NPC_LIKE)
		ai_block_damage.SetValue(false);
}
void CENPCRels::MaintainEffect()
{
	StartEffect();
}
void CENPCRels::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_NPC_HATE:
		DoNPCRels(D_HT, false);
		break;
	case EFFECT_NPC_LIKE:
		DoNPCRels(D_LI, false);
		ai_block_damage.SetValue(true);
		break;
	case EFFECT_NPC_NEUTRAL:
		DoNPCRels(D_NU, false);
		break;
	case EFFECT_NPC_FEAR:
		DoNPCRels(D_FR, false);
		break;
	}
}
void CEZombieSpam::StartEffect()
{
	char modDir[MAX_PATH];
	if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
		return;
	CAI_Node *pNode;
	CNodeList *result = GetNearbyNodes(20);
	for (; result->Count(); result->RemoveAtHead())
	{
		pNode = g_pBigAINet->GetNode(result->ElementAtHead().nodeIndex);
		//trace_t trace;
		//UTIL_TraceHull(pNode->GetPosition(HULL_HUMAN) + Vector(0, 0, 32), pNode->GetPosition(HULL_HUMAN) + Vector(0, 0, 32), NAI_Hull::Mins(HULL_HUMAN), NAI_Hull::Maxs(HULL_HUMAN), 0, NULL, COLLISION_GROUP_NONE, &trace);
		//if (trace.fraction == 1)
		//{
		int nRandom;
		if (!Q_strcmp(modDir, "ep2chaos"))
			nRandom = fmod((chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 5) : chaos_rng1.GetInt()) + result->ElementAtHead().nodeIndex, 6);
		else if (!Q_strcmp(modDir, "ep1chaos"))
			nRandom = fmod((chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 4) : chaos_rng1.GetInt()) + result->ElementAtHead().nodeIndex, 5);
		else
			nRandom = fmod((chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 3) : chaos_rng1.GetInt()) + result->ElementAtHead().nodeIndex, 4);
		if (nRandom == 0)
		{
			CBaseEntity *pNPC = (CBaseEntity *)CreateEntityByName("npc_zombie");
			pNPC->SetAbsOrigin(pNode->GetOrigin());
			pNPC->KeyValue("targetname", "l4d_zombie");
			g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
			DispatchSpawn(pNPC);
			pNPC->Activate();
			pNPC->m_bChaosSpawned = true;
			pNPC->m_bChaosPersist = true;
		}
		if (nRandom == 1)
		{
			CBaseEntity *pNPC = (CBaseEntity *)CreateEntityByName("npc_zombie_torso");
			pNPC->SetAbsOrigin(pNode->GetOrigin());
			pNPC->KeyValue("targetname", "l4d_zombie");
			g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
			DispatchSpawn(pNPC);
			pNPC->Activate();
			pNPC->m_bChaosSpawned = true;
			pNPC->m_bChaosPersist = true;
		}
		if (nRandom == 2)
		{
			CBaseEntity *pNPC = (CBaseEntity *)CreateEntityByName("npc_poisonzombie");
			pNPC->SetAbsOrigin(pNode->GetOrigin());
			pNPC->KeyValue("targetname", "l4d_zombie");
			g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
			DispatchSpawn(pNPC);
			pNPC->Activate();
			pNPC->m_bChaosSpawned = true;
			pNPC->m_bChaosPersist = true;
		}
		if (nRandom == 3)
		{
			CBaseEntity *pNPC = (CBaseEntity *)CreateEntityByName("npc_fastzombie");
			pNPC->SetAbsOrigin(pNode->GetOrigin());
			pNPC->KeyValue("targetname", "l4d_zombie");
			g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
			DispatchSpawn(pNPC);
			pNPC->Activate();
			pNPC->m_bChaosSpawned = true;
			pNPC->m_bChaosPersist = true;
		}
		//ep1
		if (nRandom == 4)
		{
			CBaseEntity *pNPC = (CBaseEntity *)CreateEntityByName("npc_zombine");
			pNPC->SetAbsOrigin(pNode->GetOrigin());
			pNPC->KeyValue("targetname", "l4d_zombie");
			g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
			DispatchSpawn(pNPC);
			pNPC->Activate();
			pNPC->m_bChaosSpawned = true;
			pNPC->m_bChaosPersist = true;
		}
		//ep2
		if (nRandom == 5)
		{
			CBaseEntity *pNPC = (CBaseEntity *)CreateEntityByName("npc_fastzombie_torso");
			pNPC->SetAbsOrigin(pNode->GetOrigin());
			pNPC->KeyValue("targetname", "l4d_zombie");
			g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
			DispatchSpawn(pNPC);
			pNPC->Activate();
			pNPC->m_bChaosSpawned = true;
			pNPC->m_bChaosPersist = true;
		}
		//}
	}
}
void CEBottle::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	QAngle vecAngles = QAngle(0, 0, 0);
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();
	CBaseAnimating *pEnt = (CBaseAnimating *)CreateEntityByName("prop_physics_override");
	pEnt->SetModel("models/props_junk/garbage_glassbottle003a.mdl");
	Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 128;
	trace_t	trDown;
	UTIL_TraceLine(vecOrigin, vecOrigin - Vector(0, 0, 1000), MASK_SOLID, pEnt, COLLISION_GROUP_DEBRIS, &trDown);
	vecOrigin = trDown.endpos + Vector(0, 0, 1);
	Vector vecLastGoodPos = vecOrigin;
	pEnt->SetAbsOrigin(vecOrigin);
	trace_t	trace;
	int i = 0;
	//try to make bottle as big as possible with whatever space is in front of player
	do {
		/*
		pEnt->SetModelScale(i + 1);
		vecOrigin += Vector(0, 0, 9);//move bottle up a bit to account for the origin being above the ground
		vecOrigin += vecForward * 0.1 * i;//move bottle away as it gets bigger, or else we'll only get to about 45x before the trace fails because it's hitting the player
		Msg("i %i vecOrigin %0.1f %0.1f %0.1f\n", i, vecOrigin.x, vecOrigin.y, vecOrigin.z);
		//NDebugOverlay::Cross3D(vecOrigin, 16, 0, 255, 0, true, 30);
		UTIL_TraceEntity(pEnt, vecOrigin, vecOrigin, MASK_SOLID, &trace);
		i++;
		*/
		//NDebugOverlay::Cross3D(pEnt->GetAbsOrigin(), 16, 0, 255, 0, true, 30);
		pEnt->SetModelScale(i + 1);
		vecLastGoodPos = pEnt->GetAbsOrigin();
		pEnt->GetUnstuck(20, UF_NO_NODE_TELEPORT);//only go up to 20 units away from previous position, instead of 500, which risks the beer spawning in some other random place, which is bad, i guess
		UTIL_TraceEntity(pEnt, pEnt->GetAbsOrigin(), pEnt->GetAbsOrigin(), MASK_SOLID, &trace);
		i++;
	} while (trace.fraction == 1 && !trace.startsolid && i < 55 && i < chaos_beer_size_limit.GetInt());//yes this i limit actually matters, or else we will create a beer so big it hits a max coord related assert and brings the whole game to a screeching halt. what the fuck.
	if (i > 1)
		pEnt->SetModelScale(i - 1);
	pEnt->SetMaxHealth(100 * i);
	pEnt->SetHealth(100 * i);
	DispatchSpawn(pEnt);
	pEnt->Activate();
	vecOrigin = vecLastGoodPos;
	pEnt->Teleport(&vecOrigin, &vecAngles, NULL);
	IPhysicsObject *pPhys = pEnt->VPhysicsGetObject();
	pPhys->EnableDrag(false);
	pPhys->SetMass(i * 10);
	if (i > 40)
		m_strHudName = MAKE_STRING("BEER I OWED YA");
}
void CEEvilNPC::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_EVIL_ALYX:
		m_iSavedChaosID = ChaosSpawnNPC("npc_alyx", MAKE_STRING("Annoying Alyx"), SPAWNTYPE_EYELEVEL_REGULAR, "models/alyx.mdl", "alyx", "weapon_alyxgun", true)->m_iChaosID;
		break;
	case EFFECT_EVIL_NORIKO:
		EvilNoriko();
		break;
	}
}
bool CEEvilNPC::CheckStrike(const CTakeDamageInfo &info)
{
	return info.GetAttacker() == GetEntityWithID(m_iSavedChaosID);
}
void CEEvilNPC::EvilNoriko()
{
	variant_t sVariant;
	sVariant.SetString(MAKE_STRING("d1_t02_Plaza_Sit02"));
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	g_iChaosSpawnCount++;
	char szName[2048];

	//crane magnet
	Vector vecOrigin = pHL2Player->RotatedOffset(Vector(0, 164, 750), true);
	QAngle vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
	CBaseEntity *pMagnet = CreateEntityByName("phys_magnet");
	pMagnet->KeyValue("model", "models/props_wasteland/cranemagnet01a.mdl");
	pMagnet->KeyValue("massScale", "1000");
	Q_snprintf(szName, sizeof(szName), "crane_magnet_%i", g_iChaosSpawnCount);
	pMagnet->KeyValue("targetname", szName);
	pMagnet->KeyValue("overridescript", "damping,0.5,rotdamping,0.2,inertia,0.3");
	pMagnet->m_bChaosSpawned = true;
	DispatchSpawn(pMagnet);
	pMagnet->Activate();
	pMagnet->Teleport(&vecOrigin, &vecAngles, NULL);
	pMagnet->GetUnstuck(500, UF_NO_NODE_TELEPORT);

	//crane
	vecOrigin = pHL2Player->RotatedOffset(Vector(634, 0, 64), true);
	vecAngles = QAngle(0, pPlayer->GetAbsAngles().y + 90, 0);
	CBaseEntity *pVehicle = CreateEntityByName("prop_vehicle_crane");
	pVehicle->KeyValue("model", "models/Cranes/crane_docks.mdl");
	pVehicle->KeyValue("solid", "6");
	pVehicle->KeyValue("magnetname", szName);
	Q_snprintf(szName, sizeof(szName), "crane%i", g_iChaosSpawnCount);
	pVehicle->KeyValue("targetname", szName);
	pVehicle->KeyValue("vehiclescript", "scripts/vehicles/crane.txt");
	pVehicle->m_bChaosSpawned = true;
	pVehicle->Teleport(&vecOrigin, &vecAngles, NULL);

	//noriko
	vecOrigin = pHL2Player->RotatedOffset(Vector(480, 85, 135), true);
	vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 180, 0);
	CBaseEntity *pNoriko = CreateEntityByName("cycler");
	pNoriko->KeyValue("model", "models/Humans/Group02/Female_04.mdl");
	pNoriko->KeyValue("targetname", "chaos_crane_driver");
	pNoriko->m_bChaosSpawned = true;
	DispatchSpawn(pNoriko);
	pNoriko->Activate();
	pNoriko->Teleport(&vecOrigin, &vecAngles, NULL);
	pNoriko->AcceptInput("SetSequence", pNoriko, pNoriko, sVariant, 0);

	//driver
	vecOrigin = pHL2Player->RotatedOffset(Vector(534, 64, 128), true);
	vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
	CAI_BaseNPC *pDriver = (CAI_BaseNPC *)CreateEntityByName("npc_cranedriver");
	pDriver->KeyValue("vehicle", szName);
	pDriver->KeyValue("releasepause", "0");
	pDriver->m_bChaosSpawned = true;
	pDriver->m_bEvil = true;
	DispatchSpawn(pDriver);
	pDriver->Activate();
	pDriver->Teleport(&vecOrigin, &vecAngles, NULL);

	//activate crane last so everything works correctly
	DispatchSpawn(pVehicle);
	pVehicle->Activate();
	pNoriko->SetParent(pVehicle);
	m_iSavedChaosID = g_iChaosSpawnCount;
}
void CERandomSong::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	const char *sSongName = "*#music/hl1_song10.mp3";
	CPASAttenuationFilter filter(pPlayer);
	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NONE;
	ep.m_nPitch = PITCH_NORM;
	int nRandom;
	char modDir[MAX_PATH];
	if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
		return;
	if (!Q_strcmp(modDir, "ep2chaos"))
		nRandom = random->RandomInt(0, 71);
	else if (!Q_strcmp(modDir, "ep1chaos"))
		nRandom = random->RandomInt(0, 58);
	else
		nRandom = random->RandomInt(0, 47);
	Msg("Song number %i\n", nRandom);
	if (nRandom == 0)
		sSongName = "*#music/hl1_song10.mp3";
	if (nRandom == 1)
		sSongName = "*#music/hl1_song11.mp3";
	if (nRandom == 2)
		sSongName = "*#music/hl1_song14.mp3";
	if (nRandom == 3)
		sSongName = "*#music/hl1_song15.mp3";
	if (nRandom == 4)
		sSongName = "*#music/hl1_song17.mp3";
	if (nRandom == 5)
		sSongName = "*#music/hl1_song19.mp3";
	if (nRandom == 6)
		sSongName = "*#music/hl1_song20.mp3";
	if (nRandom == 7)
		sSongName = "*#music/hl1_song21.mp3";
	if (nRandom == 8)
		sSongName = "*#music/hl1_song24.mp3";
	if (nRandom == 9)
		sSongName = "*#music/hl1_song25_remix3.mp3";
	if (nRandom == 10)
		sSongName = "*#music/hl1_song26.mp3";
	if (nRandom == 11)
		sSongName = "*#music/hl1_song3.mp3";
	if (nRandom == 12)
		sSongName = "*#music/hl1_song5.mp3";
	if (nRandom == 13)
		sSongName = "*#music/hl1_song6.mp3";
	if (nRandom == 14)
		sSongName = "*#music/hl1_song9.mp3";
	if (nRandom == 15)
		sSongName = "*#music/hl2_ambient_1.wav";
	if (nRandom == 16)
		sSongName = "*#music/hl2_intro.mp3";
	if (nRandom == 17)
		sSongName = "*#music/hl2_song0.mp3";
	if (nRandom == 18)
		sSongName = "*#music/hl2_song1.mp3";
	if (nRandom == 19)
		sSongName = "*#music/hl2_song2.mp3";
	if (nRandom == 20)
		sSongName = "*#music/hl2_song3.mp3";
	if (nRandom == 21)
		sSongName = "*#music/hl2_song4.mp3";
	if (nRandom == 22)
		sSongName = "*#music/hl2_song6.mp3";
	if (nRandom == 23)
		sSongName = "*#music/hl2_song7.mp3";
	if (nRandom == 24)
		sSongName = "*#music/hl2_song8.mp3";
	if (nRandom == 25)
		sSongName = "*#music/hl2_song10.mp3";
	if (nRandom == 26)
		sSongName = "*#music/hl2_song11.mp3";
	if (nRandom == 27)
		sSongName = "*#music/hl2_song12_long.mp3";
	if (nRandom == 28)
		sSongName = "*#music/hl2_song13.mp3";
	if (nRandom == 29)
		sSongName = "*#music/hl2_song14.mp3";
	if (nRandom == 30)
		sSongName = "*#music/hl2_song15.mp3";
	if (nRandom == 31)
		sSongName = "*#music/hl2_song16.mp3";
	if (nRandom == 32)
		sSongName = "*#music/hl2_song17.mp3";
	if (nRandom == 33)
		sSongName = "*#music/hl2_song19.mp3";
	if (nRandom == 34)
		sSongName = "*#music/hl2_song20_submix0.mp3";
	if (nRandom == 35)
		sSongName = "*#music/hl2_song20_submix4.mp3";
	if (nRandom == 36)
		sSongName = "*#music/hl2_song23_suitsong3.mp3";
	if (nRandom == 37)
		sSongName = "*#music/hl2_song26.mp3";
	if (nRandom == 38)
		sSongName = "*#music/hl2_song26_trainstation1.mp3";
	if (nRandom == 39)
		sSongName = "*#music/hl2_song27_trainstation2.mp3";
	if (nRandom == 40)
		sSongName = "*#music/hl2_song28.mp3";
	if (nRandom == 41)
		sSongName = "*#music/hl2_song29.mp3";
	if (nRandom == 42)
		sSongName = "*#music/hl2_song30.mp3";
	if (nRandom == 43)
		sSongName = "*#music/hl2_song31.mp3";
	if (nRandom == 44)
		sSongName = "*#music/hl2_song32.mp3";
	if (nRandom == 45)
		sSongName = "*#music/hl2_song33.mp3";
	if (nRandom == 46)
		sSongName = "*#music/radio1.mp3";
	if (nRandom == 47)
		sSongName = "*#music/ravenholm_1.mp3";
	//ep1
	if (nRandom == 48)
		sSongName = "*#music/vlvx_song1.mp3";
	if (nRandom == 49)
		sSongName = "*#music/vlvx_song2.mp3";
	if (nRandom == 50)
		sSongName = "*#music/vlvx_song4.mp3";
	if (nRandom == 51)
		sSongName = "*#music/vlvx_song8.mp3";
	if (nRandom == 52)
		sSongName = "*#music/vlvx_song11.mp3";
	if (nRandom == 53)
		sSongName = "*#music/vlvx_song12.mp3";
	if (nRandom == 54)
		sSongName = "*#music/vlvx_song18.mp3";
	if (nRandom == 55)
		sSongName = "*#music/vlvx_song19a.mp3";
	if (nRandom == 56)
		sSongName = "*#music/vlvx_song19b.mp3";
	if (nRandom == 57)
		sSongName = "*#music/vlvx_song20.mp3";
	if (nRandom == 58)
		sSongName = "*#music/vlvx_song21.mp3";
	//ep2
	if (nRandom == 59)
		sSongName = "*#music/vlvx_song0.mp3";
	if (nRandom == 60)
		sSongName = "*#music/vlvx_song3.mp3";
	if (nRandom == 61)
		sSongName = "*#music/vlvx_song9.mp3";
	if (nRandom == 62)
		sSongName = "*#music/vlvx_song15.mp3";
	if (nRandom == 63)
		sSongName = "*#music/vlvx_song20.mp3";
	if (nRandom == 64)
		sSongName = "*#music/vlvx_song22.mp3";
	if (nRandom == 65)
		sSongName = "*#music/vlvx_song23.mp3";
	if (nRandom == 66)
		sSongName = "*#music/vlvx_song23ambient.mp3";
	if (nRandom == 67)
		sSongName = "*#music/vlvx_song24.mp3";
	if (nRandom == 68)
		sSongName = "*#music/vlvx_song25.mp3";
	if (nRandom == 69)
		sSongName = "*#music/vlvx_song26.mp3";
	if (nRandom == 70)
		sSongName = "*#music/vlvx_song27.mp3";
	if (nRandom == 71)
		sSongName = "*#music/vlvx_song28.mp3";
	ep.m_pSoundName = sSongName;
	pPlayer->PrecacheSound(sSongName);//because precaching every single song on spawn is not the winning move
	pPlayer->EmitSound(filter, pPlayer->entindex(), ep);
}
void CETreeSpam::StartEffect()
{
	CAI_Node *pNode;
	CNodeList *result = GetNearbyNodes(80);
	Msg("list has %i nodes\n", result->Count());
	CUtlVector<Vector> vecTreeSpots;//positions of trees we've placed, or other places we'd like to avoid
	vecTreeSpots.AddToTail(UTIL_GetLocalPlayer()->GetAbsOrigin());
	//track doors because often nodes are placed around doorways, and trees often block necessary doorways
	CBaseEntity *pDoor = gEntList.FindEntityByClassname(NULL, "prop_door_rotating");
	while (pDoor)
	{
		vecTreeSpots.AddToTail(pDoor->GetAbsOrigin());
		pDoor = gEntList.FindEntityByClassname(pDoor, "prop_door_rotating");
	}

	for (; result->Count(); result->RemoveAtHead())
	{
		pNode = g_pBigAINet->GetNode(result->ElementAtHead().nodeIndex);
		Msg("node %i\n", pNode->GetId());
		CBaseEntity *pEnt = CreateEntityByName("prop_dynamic");
		trace_t tr;
		Vector vecNodePos = pNode->GetOrigin();
		UTIL_TraceLine(vecNodePos + Vector(0, 0, 16), vecNodePos - Vector(0, 0, 100), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);
		if (tr.m_pEnt && (tr.m_pEnt->GetMoveType() == MOVETYPE_VPHYSICS || tr.m_pEnt->IsNPC()))
		{
			Msg("Tree (node %i) on bad ground\n", pNode->GetId());
			continue;
		}
		bool bDone = false;
		for (int i = 0; vecTreeSpots.Size() >= i + 1; i++)
		{
			if ((vecTreeSpots[i] - tr.endpos).Length2D() < 90)
			{
				bDone = true;
				break;
			}
		}
		if (bDone)
			continue;
		pEnt->SetAbsOrigin(tr.endpos);
		vecTreeSpots.AddToTail(pEnt->GetAbsOrigin());
		pEnt->KeyValue("model", "models/props_foliage/tree_pine04.mdl");
		pEnt->KeyValue("disableshadows", "1");//shadows may cause a surprising amount of lag
		pEnt->KeyValue("solid", "6");
		g_iChaosSpawnCount++; pEnt->KeyValue("chaosid", g_iChaosSpawnCount);
		pEnt->m_bChaosPersist = true;
		pEnt->m_bChaosSpawned = true;
		DispatchSpawn(pEnt);
		pEnt->Activate();
	}
	UTIL_GetLocalPlayer()->GetUnstuck(500, UF_NO_NODE_TELEPORT);//despite earlier check, we can still end up stuck in a tree if there are multiple nodes very closeby
}
void CETreeSpam::StopEffect()
{
	CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, "prop_dynamic");
	while (pEnt)
	{
		if (pEnt->m_bChaosPersist && !Q_strcmp(STRING(pEnt->GetModelName()), "models/props_foliage/tree_pine04.mdl"))
			pEnt->Remove();
		pEnt = gEntList.FindEntityByClassname(pEnt, "prop_dynamic");
	}
}
void CEMountedGun::StartEffect()
{
	CBaseEntity *pTank;
	//float flDistAway = 128;
	//float flExtraHeight = 32;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();
	Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 128 + Vector(0, 0, 16);
	trace_t tr;
	UTIL_TraceLine(UTIL_GetLocalPlayer()->EyePosition(), vecOrigin, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
	Vector vecTargetOrigin = tr.endpos - vecForward * 40;
	QAngle vecAngles(0, pPlayer->GetAbsAngles().y, 0);
	//pTank->SetAbsOrigin(vecOrigin);
	//pTank->SetAbsAngles(vecAngles);
	vecOrigin = tr.endpos - vecForward * 5 + Vector(0, 0, 32);//move back out of walls so we're guaranteed to be usable
	CBaseEntity *pProp = CreateEntityByName("prop_dynamic");
	if (pProp)
	{
		pProp->KeyValue("targetname", "gunmodel");
		pProp->KeyValue("DefaultAnim", "idle_inactive");
		pProp->KeyValue("parentname", "tank");
	}
	CBaseEntity *pTarget = CreateEntityByName("info_target");
	if (pTarget)
	{
		pTarget->KeyValue("targetname", "tank_npc_spot");
		pTarget->KeyValue("targetname", "gunmodel");
		DispatchSpawn(pTarget);
		pTarget->Activate();
		pTarget->Teleport(&vecTargetOrigin, &vecAngles, NULL);
	}
	int nRandom;
	char modDir[MAX_PATH];
	if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
		return;
	if (!Q_strcmp(modDir, "ep2chaos"))
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 4) : chaos_rng1.GetInt();
	else
		nRandom = chaos_rng1.GetInt() == -1 ? random->RandomInt(0, 3) : chaos_rng1.GetInt();
	if (nRandom == 0)
	{
		pTank = CreateEntityByName("func_tank");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 15);
			pTank->KeyValue("bullet_damage", 3);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("bullet", 3);
			pTank->KeyValue("effecthandling", 1);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("ammotype", "AR2");
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", 8);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pProp->KeyValue("rendercolor", "255 255 255");
			pProp->KeyValue("model", "models/props_combine/bunker_gun01.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	if (nRandom == 1)
	{
		pTank = CreateEntityByName("func_tankairboatgun");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 66);
			pTank->KeyValue("bullet_damage", 250);
			pTank->KeyValue("bullet_damage_vs_player", 25);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", "0");
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pProp->KeyValue("rendercolor", "255 255 255");
			pProp->KeyValue("model", "models/Airboatgun.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	if (nRandom == 2)
	{
		pTank = CreateEntityByName("func_tankmortar");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 0.22);
			pTank->KeyValue("bullet_damage", 0.0);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", 8);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pTank->KeyValue("spriteflash", "materials/Sprites/redglow1.vmt");
			pTank->KeyValue("firedelay", 1.5);
			pTank->KeyValue("firestartsound", "Weapon_Mortar.Single");
			pTank->KeyValue("incomingsound", "Weapon_Mortar.Incomming");
			pTank->KeyValue("warningtime", 1);
			pTank->KeyValue("firevariance", 1);
			pProp->KeyValue("rendercolor", "255 255 0");
			pProp->KeyValue("model", "models/props_combine/bunker_gun01.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	if (nRandom == 3)
	{
		pTank = CreateEntityByName("func_tankrocket");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 1.5);
			pTank->KeyValue("bullet_damage", 100);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("bullet", 3);
			pTank->KeyValue("effecthandling", 0.0);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("ammotype", "AR2");
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", 8);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pTank->KeyValue("rocketspeed", "1500");
			pProp->KeyValue("rendercolor", "255 0 0");
			pProp->KeyValue("model", "models/props_combine/bunker_gun01.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	//ep2
	if (nRandom == 4)
	{
		pTank = CreateEntityByName("func_tank_combine_cannon");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 15);
			pTank->KeyValue("bullet_damage", 10);
			pTank->KeyValue("bullet_damage_vs_player", 25);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("bullet", 3);
			pTank->KeyValue("effecthandling", 2);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("ammotype", "CombineHeavyCannon");
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 20);
			pTank->KeyValue("barrelz", 13);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pTank->KeyValue("manual", true);
			pProp->KeyValue("rendercolor", "255 255 255");
			pProp->KeyValue("model", "models/combine_turrets/combine_cannon_gun.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
}
void CERestartLevel::StartEffect()
{
	g_bGoBackLevel = true;
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "restart");
}
void CERemovePickups::StartEffect()
{
	CBaseEntity *pPickup = gEntList.FindEntityByClassname(NULL, "it*");
	while (pPickup)
	{
		pPickup->Remove();
		pPickup = gEntList.FindEntityByClassname(pPickup, "it*");
	}
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon *)gEntList.FindEntityByClassname(NULL, "we*");
	while (pWeapon)
	{
		if (!pWeapon->GetOwner())
			pWeapon->Remove();
		pWeapon = (CBaseCombatWeapon *)gEntList.FindEntityByClassname(pWeapon, "we*");
	}
}
void CECloneNPCs::StartEffect()
{
	CUtlVector<CChaosStoredEnt> vNPCs;
	CBaseEntity *pNPC = gEntList.FindEntityByClassname(NULL, "n*");
	while (pNPC)
	{
		//avoid cloning non-NPC entities with an "npc_" prefix, like npc_maker
		//avoid npc_furniture, it goes bad
		if (pNPC->IsNPC() && !pNPC->ClassMatches("npc_furniture"))
			vNPCs.AddToTail(*StoreEnt(pNPC));
		pNPC = gEntList.FindEntityByClassname(pNPC, "n*");
	}
	for (int i = 0; vNPCs.Size() >= i + 1; i++)
	{
		CBaseEntity *pCloneNPC = RetrieveStoredEnt(&vNPCs[i], false);
		if (pCloneNPC)
		{
			Vector vecOrigin = pCloneNPC->GetAbsOrigin();
			QAngle vecAngle = pCloneNPC->GetAbsAngles();
			DispatchSpawn(pCloneNPC);
			pCloneNPC->Activate();
			pCloneNPC->Teleport(&vecOrigin, &vecAngle, NULL);
		}
	}
}
void CEBumpy::FastThink()
{
	if (IterUsableVehicles(false))
		m_bReverse = !m_bReverse;
}
void CEBumpy::DoOnVehicles(CPropVehicleDriveable *pVehicle)
{
	int iSpheres = 0;
	CBaseEntity *pSphere = gEntList.FindEntityByClassname(NULL, "prop_physics");
	while (pSphere)
	{
		if (pSphere->m_bChaosSpawned && !strcmp(STRING(pSphere->GetModelName()), "models/props_c17/oildrum001.mdl"))
		{
			iSpheres++;
			//get rid of props that are far away
			if ((pSphere->GetAbsOrigin() - pVehicle->GetAbsOrigin()).Length2D() > 300 || !pVehicle->GetDriver())
			{
				UTIL_Remove(pSphere);
				iSpheres--;
			}
		}
		pSphere = gEntList.FindEntityByClassname(pSphere, "prop_physics");
	}
	if (iSpheres < 7 && pVehicle->GetDriver())
	{
		//put invisible props in front of the car
		QAngle angFacing = pVehicle->GetAbsAngles();
		float flSidewaysPos = RandomInt(-50, 50);
		float flForwardPos = RandomInt(pVehicle->GetPhysics()->GetVehicleOperatingParams().speed >= 0 ? 200 : -200, pVehicle->GetPhysics()->GetVehicleOperatingParams().speed >= 0 ? 300 : -300);
		Vector vecOffset = Vector(flSidewaysPos, flForwardPos, 0);
		Vector vecRotated;
		VectorRotate(vecOffset, angFacing, vecRotated);
		trace_t tr;
		UTIL_TraceLine(vecRotated + pVehicle->GetAbsOrigin(), vecRotated + pVehicle->GetAbsOrigin() - Vector(0, 0, 1000), MASK_SOLID, pVehicle, COLLISION_GROUP_DEBRIS, &tr);
		CBaseEntity *pEnt = CreateEntityByName("prop_physics");
		pEnt->SetAbsOrigin(tr.endpos - Vector(0, 0, 7));
		pEnt->SetAbsAngles(QAngle(90, angFacing.y, 0));
		pEnt->SetModel("models/props_c17/oildrum001.mdl");
		pEnt->AddEffects(EF_NODRAW);
		g_iChaosSpawnCount++;
		pEnt->m_iChaosID = g_iChaosSpawnCount;
		pEnt->m_bChaosSpawned = true;
		pEnt->AddSpawnFlags(8);
		DispatchSpawn(pEnt);
	}
}
void CEGravitySet::StartEffect()
{
	bool bNegative = g_ChaosEffects[EFFECT_INVERTG]->m_bActive;
	switch (m_nID)
	{
	case EFFECT_ZEROG:
		sv_gravity.SetValue(0);
		Msg("Setting sv_gravity to 0\n");
		break;
	case EFFECT_SUPERG:
		sv_gravity.SetValue(bNegative ? -1800 : 1800);
		Msg("Setting sv_gravity to %i\n", bNegative ? -1800 : 1800);
		break;
	case EFFECT_LOWG:
		sv_gravity.SetValue(bNegative ? -200 : 200);
		Msg("Setting sv_gravity to %i\n", bNegative ? -200 : 200);
		break;
	case EFFECT_INVERTG:
		Msg("Setting sv_gravity to %i\n", -sv_gravity.GetInt());
		sv_gravity.SetValue(-sv_gravity.GetInt());
		break;
	}
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
void CEGravitySet::StopEffect()
{
	bool bNegative = g_ChaosEffects[EFFECT_INVERTG]->m_bActive;
	switch (m_nID)
	{
	case EFFECT_ZEROG:
	case EFFECT_SUPERG:
	case EFFECT_LOWG:
		sv_gravity.SetValue(bNegative ? -600 : 600);
		Msg("Unsetting sv_gravity to %i\n", bNegative ? -600 : 600);
		break;
	case EFFECT_INVERTG:
		Msg("Setting sv_gravity to %i\n", -sv_gravity.GetInt());
		sv_gravity.SetValue(-sv_gravity.GetInt());
		break;
	}
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
bool CEGravitySet::CheckStrike(const CTakeDamageInfo &info)
{
	return (info.GetDamageType() & DMG_FALL) != 0;
}
void CEGravitySet::MaintainEffect()
{
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
void CEPhysSpeedSet::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_PHYS_PAUSE:
		phys_timescale.SetValue(0);
		break;
	case EFFECT_PHYS_FAST:
		phys_timescale.SetValue(4);
		break;
	case EFFECT_PHYS_SLOW:
		phys_timescale.SetValue(0.25f);
		break;
	}
}
void CEPhysSpeedSet::StopEffect()
{
	phys_timescale.SetValue(1);
}
void CEStop::StartEffect()
{
	sv_maxspeed.SetValue(0);
	IterUsableVehicles(false);
}
void CEStop::DoOnVehicles(CPropVehicleDriveable *pVehicle)
{
	variant_t emptyVariant;
	if (m_flTimeRem < 1)
		pVehicle->AcceptInput("TurnOn", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), emptyVariant, 0);
	else
		pVehicle->AcceptInput("TurnOff", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), emptyVariant, 0);
}
void CEStop::StopEffect()
{
	sv_maxspeed.SetValue(320);
	UTIL_GetLocalPlayer()->SetMaxSpeed(HL2_NORM_SPEED);
	IterUsableVehicles(false);
	if (g_ChaosEffects[EFFECT_SUPER_MOVEMENT]->m_bActive && g_ChaosEffects[EFFECT_SUPER_MOVEMENT]->m_flTimeRem > 1)
	{
		//EFFECT_SUPER_MOVEMENT is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_SUPER_MOVEMENT]->StartEffect();
	}
}
void CESwimInAir::StartEffect()
{
	UTIL_GetLocalPlayer()->m_bSwimInAir = true;
}
void CESwimInAir::StopEffect()
{
	UTIL_GetLocalPlayer()->m_bSwimInAir = false;
}
void CESwimInAir::FastThink()
{
	//TODO: can we move this to MaintainEffect? it's just been here since creation.
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer->IsInAVehicle())
	{
		pPlayer->SetWaterLevel(WL_Eyes);
		pPlayer->SetPlayerUnderwater(true);
		pPlayer->AddFlag(FL_INWATER);
		pPlayer->AddFlag(FL_SWIM);
	}
	//other half of this logic is in prethink
}
bool CESwimInAir::CheckStrike(const CTakeDamageInfo &info)
{
	return (info.GetDamageType() & DMG_DROWN) != 0;
}
void CELockPVS::StartEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_lockpvs 0; exec portalsopenall; wait 100; r_lockpvs 1; r_portalsopenall 1\n");
}
void CELockPVS::StopEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_lockpvs 0; exec portalsopenall\n");
}
void CELockPVS::TransitionEffect()
{
	StartEffect();
}
void CEPlayerBig::StartEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(2);
	UTIL_GetLocalPlayer()->GetUnstuck(500);//done here so that the player won't be stuck when reloading a save that was made before the effect was on
}
void CEPlayerBig::StopEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(1);
	if (g_ChaosEffects[EFFECT_PLAYER_SMALL]->m_bActive && g_ChaosEffects[EFFECT_PLAYER_SMALL]->m_flTimeRem > 1)
	{
		//EFFECT_PLAYER_SMALL is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_PLAYER_SMALL]->StartEffect();
	}
}
void CEPlayerBig::MaintainEffect()
{
	//if (!UTIL_GetLocalPlayer()->IsInAVehicle())
	//	UTIL_GetLocalPlayer()->GetUnstuck(500);
}
void CEPlayerSmall::StartEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(0.5);
}
void CEPlayerSmall::StopEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(1);
	UTIL_GetLocalPlayer()->GetUnstuck(500);
	if (g_ChaosEffects[EFFECT_PLAYER_BIG]->m_bActive && g_ChaosEffects[EFFECT_PLAYER_BIG]->m_flTimeRem > 1)
	{
		//EFFECT_PLAYER_BIG is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_PLAYER_BIG]->StartEffect();
	}
}
void CEPlayerSmall::MaintainEffect()
{
	if (!UTIL_GetLocalPlayer()->IsInAVehicle())
		UTIL_GetLocalPlayer()->GetUnstuck(500);
}
void CESuperGrab::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	player_use_dist.SetValue(8000);
	player_throwforce.SetValue(50000);
	if (pHL2Player)
		pHL2Player->m_bSuperGrab = true;
}
void CESuperGrab::StopEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	player_use_dist.SetValue(80);
	player_throwforce.SetValue(1000);
	if (pHL2Player)
		pHL2Player->m_bSuperGrab = false;
}
void CEWeaponsDrop::StartEffect()
{
	m_bDone = false;
}
void CEWeaponsDrop::FastThink()
{
	if (m_bDone)
		return;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CBaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();
	QAngle gunAngles;
	VectorAngles(pPlayer->BodyDirection2D(), gunAngles);
	Vector vecForward;
	AngleVectors(gunAngles, &vecForward, NULL, NULL);
	float flDiameter = sqrt(pPlayer->CollisionProp()->OBBSize().x * pPlayer->CollisionProp()->OBBSize().x +
		pPlayer->CollisionProp()->OBBSize().y * pPlayer->CollisionProp()->OBBSize().y);
	for (int i = 0; i<MAX_WEAPONS; ++i)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->m_hMyWeapons[i];
		if (!pWeapon)
			continue;
		// Have to drop this after we've dropped everything else, so autoswitch doesn't happen
		if (pWeapon == pActiveWeapon)
			continue;
		pPlayer->DropWeaponForWeaponStrip(pWeapon, vecForward, gunAngles, flDiameter);

		//a little speculative fix. we had an unexplainable engine crash once when dropping all weapons simultaneously
		//so now we're dropping one per tick to hopefully fix that
		return;
	}
	m_bDone = true;
	// Drop the active weapon normally...
	if (pActiveWeapon)
	{
		// Nowhere in particular; just drop it.
		Vector vecThrow;
		pPlayer->ThrowDirForWeaponStrip(pActiveWeapon, vecForward, &vecThrow);
		// Throw a little more vigorously; it starts closer to the player
		vecThrow *= random->RandomFloat(800.0f, 1000.0f);
		pPlayer->Weapon_Drop(pActiveWeapon, NULL, &vecThrow);
		pActiveWeapon->SetRemoveable(false);
	}
}
void CEEarthquake::StartEffect()
{
	UTIL_ScreenShake(UTIL_GetLocalPlayer()->WorldSpaceCenter(), 50 * UTIL_GetLocalPlayer()->GetModelScale(), 2, m_flTimeRem, 375, SHAKE_START, true);
}
void CEEarthquake::TransitionEffect()
{
	StartEffect();
}
void CE420Joke::StartEffect()
{
	UTIL_GetLocalPlayer()->SetHealth(4);
	UTIL_GetLocalPlayer()->SetArmorValue(20);
}
void CEQuickclip::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_QUICKCLIP_ON:
		UTIL_GetLocalPlayer()->SetCollisionGroup(COLLISION_GROUP_IN_VEHICLE);
		break;
	case EFFECT_QUICKCLIP_OFF:
		UTIL_GetLocalPlayer()->SetCollisionGroup(COLLISION_GROUP_PLAYER);
		break;
	}
}
void CEFloorIsLava::FastThink()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	bool bSkipThisTick = false;
	//have to be on solid ground
	if (pPlayer->IsInAVehicle())
		bSkipThisTick = true;

	//don't hurt if in water
	if (pPlayer->GetWaterLevel() >= WL_Feet)
		bSkipThisTick = true;

	//allow all grating and clipping
	if (pPlayer->m_chTextureType == 'G' || pPlayer->m_chTextureType == 'I')
		bSkipThisTick = true;

	if (!bSkipThisTick)
	{
		//trace hull lets us see when player is surfing. checking ground entity can't differentiate between surfing and being entirely in the air
		float flHullWidth = 16 * pPlayer->GetModelScale();
		trace_t	trace;
		UTIL_TraceHull(pPlayer->GetAbsOrigin() + Vector(0, 0, 1), pPlayer->GetAbsOrigin() - Vector(0, 0, 1), Vector(-flHullWidth, -flHullWidth, -1), Vector(flHullWidth, flHullWidth, 0), CONTENTS_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace);

		//have to be on solid ground
		//all entities get a free pass
		if (!trace.m_pEnt || !trace.m_pEnt->IsWorld())
			bSkipThisTick = true;

		if (!bSkipThisTick)
		{
			//don't count static props
			if (!strcmp(trace.surface.name, "**studio**"))
				bSkipThisTick = true;
		}

		//test in center and 4 corners
		trace_t	trace2;
		//Vector vecCorners[5] = { Vector(0, 0, 0), Vector(-16, -16, 0), Vector(16, -16, 0), Vector(-16, 16, 0), Vector(16, 16, 0) };
		//for (int i = 0; i < 5; i++)
		//{
			UTIL_TraceLine(pPlayer->GetAbsOrigin(), pPlayer->GetAbsOrigin() - Vector(0, 0, 20), CONTENTS_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace2);

			//if you're on sky or nodraw, then whatever
			if ((trace2.surface.flags & SURF_SKY) || (trace2.surface.flags & SURF_NODRAW))
			{
				bSkipThisTick = true;
				//break;
			}
		//}
	}

	if (bSkipThisTick)
	{
		//off ground, reset the timer so that we get burned on the first tick we're back on solid ground
		m_iSkipTicks = 0;
	}
	else
	{
		//When on solid ground, wait X ticks between applying damage.
		if (m_iSkipTicks > 0)
		{
			//tick timer down
			m_iSkipTicks--;
			return;
		}
		else
		{
			//time to burn again
			m_iSkipTicks = 7;
			//apply dmg
			CTakeDamageInfo info(pPlayer, pPlayer, 1, DMG_BURN);
			pPlayer->TakeDamage(info);
		}
	}
}
bool CEFloorIsLava::CheckStrike(const CTakeDamageInfo &info)
{
	return (info.GetDamageType() & DMG_BURN) != 0;
}
void CEUseSpam::MaintainEffect()
{
	//apparently maintain effect can become desynced or something, so yeah
	//if (m_flLastUseThink < gpGlobals->curtime)
	//{
	//i don't know why, but the wait amounts don't match up with what really happens. despite this command string looking like it should last for 4 seconds, it only lasts for 1
		engine->ClientCommand(engine->PEntityOfEntIndex(1),
			"+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;+use;wait 5;-use;wait 5;\n");
	//	m_flLastUseThink = gpGlobals->curtime + 1;
	//}
}
void CENoBrake::StartEffect()
{
	r_handbrake_allowed.SetValue(0);
	r_vehicleBrakeRate.SetValue(0);
}
void CENoBrake::StopEffect()
{
	r_handbrake_allowed.SetValue(1);
	r_vehicleBrakeRate.SetValue(1.5f);
}
void CEForceInOutCar::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CPropVehicleDriveable *pVehicle = (CPropVehicleDriveable *)pPlayer->GetVehicleEntity();
	if (pVehicle)
	{
		//get out
		bool bWasLocked = pVehicle->IsLocked();
		pVehicle->SetLocked(false);
		pPlayer->GetVehicle()->HandlePassengerExit(pPlayer);
		pVehicle->SetLocked(bWasLocked);
		m_strHudName = MAKE_STRING("Force Out of Vehicle");
	}
	else
	{
		//get in
		IterUsableVehicles(false);
	}
	
}
void CEForceInOutCar::DoOnVehicles(CPropVehicleDriveable *pVehicle)
{
	if (m_bFoundOne)
		return;
	//find a vehicle first
	if (pVehicle)
	{
		UTIL_GetLocalPlayer()->ForceDropOfCarriedPhysObjects();
		bool bWasLocked = pVehicle->IsLocked();
		pVehicle->SetLocked(false);
		pVehicle->GetServerVehicle()->HandlePassengerEntry(UTIL_GetLocalPlayer(), true);
		pVehicle->SetLocked(bWasLocked);
		m_strHudName = MAKE_STRING("Force Into Vehicle");
		m_bFoundOne = true;
	}
}
void CEWeaponRemove::StartEffect()
{
	if (UTIL_GetLocalPlayer()->GetActiveWeapon() == NULL)
		return;
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon *)gEntList.FindEntityByClassname(NULL, "we*");
	while (pWeapon)
	{
		if (pWeapon->GetOwner() && pWeapon->GetOwner()->IsPlayer())
			break;
		pWeapon = (CBaseCombatWeapon *)gEntList.FindEntityByClassname(pWeapon, "we*");
	}
	if (!pWeapon)
		return;
	//hide model
	CBaseViewModel *vm = UTIL_GetLocalPlayer()->GetViewModel(pWeapon->m_nViewModelIndex);
	if (vm && pWeapon == UTIL_GetLocalPlayer()->GetActiveWeapon())
		vm->AddEffects(EF_NODRAW);
	pWeapon->Delete();
}
void CEPhysConvert::StartEffect()
{
	//door-linked areaportals become permanently open since the door is now free to move
	CBaseEntity *pPortal = NULL;
	while ((pPortal = gEntList.FindEntityByClassname(pPortal, "func_a*")) != NULL)
	{
		if (pPortal->m_target != NULL_STRING)
		{
			// USE_ON means open the portal, off means close it
			pPortal->Use(pPortal, pPortal, USE_ON, 0);
			pPortal->m_target = NULL_STRING;
		}
	}
	CBaseEntity *pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		//disable physics constraints and break ropes for maximum breaking
		if (pEnt->ClassMatches("ph*") || pEnt->ClassMatches("mov*") || pEnt->ClassMatches("k*"))
		{
			variant_t emptyVariant;
			pEnt->AcceptInput("Break", pEnt, pEnt, emptyVariant, 0);
			pEnt->AcceptInput("Turnoff", pEnt, pEnt, emptyVariant, 0);//Also hit phys_thruster and torque
			pEnt = gEntList.NextEnt(pEnt);
			continue;
		}
		//tell crane drivers that their magnet just dropped stuff, or they may act odd
		if (pEnt->ClassMatches("npc_cra*"))
		{
			variant_t emptyVariant;
			pEnt->AcceptInput("MagnetDropped", pEnt, pEnt, emptyVariant, 0);
			pEnt = gEntList.NextEnt(pEnt);
			continue;
		}
		//objects have to be real "things"
		//no world
		//no vehicles
		//no players
		//no children (some props are visual representations of invisible brush entities)
		//and don't be an invisible brush entity or something along those lines
		if (pEnt->IsEffectActive(EF_NODRAW) || !(pEnt->GetSolid() == SOLID_BSP || pEnt->GetSolid() == SOLID_VPHYSICS) || pEnt->IsSolidFlagSet(FSOLID_NOT_SOLID) ||
			pEnt->IsWorld() || pEnt->IsPlayer() ||
			!pEnt->GetModel() || pEnt->GetServerVehicle() || pEnt->GetParent())
		{
			pEnt = gEntList.NextEnt(pEnt);
			continue;
		}
		//we DO let vphysics entities pass because we still want to wake them

		IPhysicsObject *pPhysicsObject = pEnt->VPhysicsGetObject();
		if (pPhysicsObject && pPhysicsObject->IsStatic())
		{
			pEnt->VPhysicsDestroyObject();
			pEnt->VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);
			pPhysicsObject = pEnt->VPhysicsGetObject();
		}
		if (!pPhysicsObject)
			pPhysicsObject = pEnt->VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);
		if (pPhysicsObject != NULL)
		{
			pPhysicsObject->SetShadow(1e4, 1e4, true, true);
			pEnt->SetSolid(SOLID_VPHYSICS);
			pEnt->SetMoveType(MOVETYPE_VPHYSICS);
			pPhysicsObject->RemoveShadowController();
			pEnt->VPhysicsUpdate(pPhysicsObject);
			pPhysicsObject->EnableMotion(true);
			pPhysicsObject->RecheckCollisionFilter();
			pPhysicsObject->Wake();
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
}
void CEIncline::StartEffect()
{
	steepness_limit.SetValue(0.96f);//1.0 is too aggressive, we will slide around on seemingly flat things
	sv_airaccelerate.SetValue(1);
	UTIL_GetLocalPlayer()->m_Local.m_flStepSize = 0;
	chaos_disable_ladders.SetValue(true);
}
void CEIncline::StopEffect()
{
	steepness_limit.SetValue(0.7f);
	sv_airaccelerate.SetValue(10);
	UTIL_GetLocalPlayer()->m_Local.m_flStepSize = 18;
	chaos_disable_ladders.SetValue(false);
}
void CEDeathWater::FastThink()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer->GetWaterLevel() >= WL_Feet && !pPlayer->IsInAVehicle())
	{
		CTakeDamageInfo info(pPlayer, pPlayer, 1000, DMG_SONIC);
		pPlayer->TakeDamage(info);
	}
}
bool CEDeathWater::CheckStrike(const CTakeDamageInfo &info)
{
	return (info.GetDamageType() & DMG_SONIC) != 0;
}
void CEBarrelShotgun::StartEffect()
{
	chaos_barrel_shotgun.SetValue(1);
}
void CEBarrelShotgun::StopEffect()
{
	chaos_barrel_shotgun.SetValue(0);
}
bool CEBarrelShotgun::CheckStrike(const CTakeDamageInfo &info)
{
	return !strcmp(STRING(info.GetAttacker()->GetModelName()), "models/props_c17/oildrum001_explosive.mdl") && info.GetAttacker()->m_bChaosSpawned;
}
void CEDejaVu::StartEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "reload setpos");
}
void CEDejaVu::MaintainEffect()
{
	if (!m_bDone)
		UTIL_GetLocalPlayer()->GetUnstuck(500);
	m_bDone = true;
}
void CERandomCC::StartEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "cc_generate");
}
void CERandomCC::StopEffect()
{
	CBaseEntity *pCC = gEntList.FindEntityByName(NULL, "chaos_cc");
	while (pCC)
	{
		variant_t emptyVariant;
		UTIL_Remove(pCC);
		pCC = gEntList.FindEntityByName(pCC, "chaos_cc");
	}
}
void CERandomCC::TransitionEffect()
{
	StopEffect();
	DisplayGeneratedCC();
}
void CERandomCC::RestoreEffect()
{
	StopEffect();
	DisplayGeneratedCC();
#endif