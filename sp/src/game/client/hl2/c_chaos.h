#ifndef C_CHAOS_H
#define C_CHAOS_H
#pragma once

#include "cbase.h"

class C_ChaosControllerProxy : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_ChaosControllerProxy, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	static void NotifyNetworkStateChanged();
	int UpdateTransmitState();
	static C_ChaosControllerProxy *s_pChaosControllerProxy;
};

class C_ChaosEffect
{
public:
	DECLARE_CLASS_NOBASE( C_ChaosEffect );
	DECLARE_CLIENTCLASS_NOBASE();

	float GetTimeLeftFraction() { return Clamp(m_flTimeRem, 0.f, 1.f); }

	char m_strName[MAX_EFFECT_NAME];
	float m_flTimeRem;
};

class C_ChaosController
{
public:
	DECLARE_CLASS_NOBASE( C_ChaosController );
	DECLARE_CLIENTCLASS_NOBASE();
	
	// This function is here for our CNetworkVars.
	inline void NetworkStateChanged()
	{
		// Forward the call to the entity that will send the data.
		C_ChaosControllerProxy::NotifyNetworkStateChanged();
	}

	inline void NetworkStateChanged( void *pVar )
	{
		// Forward the call to the entity that will send the data.
		C_ChaosControllerProxy::NotifyNetworkStateChanged();
	}

	CUtlVector<C_ChaosEffect> m_activeEffects;
	float m_flNextEffectRem;
};

extern C_ChaosController g_CC;

#endif
