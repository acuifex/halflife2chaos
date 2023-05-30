#include "c_chaos.h"

#include "dt_utlvector_recv.h"

C_ChaosController g_CC;

BEGIN_RECV_TABLE_NOBASE( C_ChaosEffect, DT_ChaosEffect )
	RecvPropString( RECVINFO( m_strName ) ),
	RecvPropFloat( RECVINFO( m_flTimeRem ) ),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_ChaosController, DT_ChaosController )
	RecvPropFloat( RECVINFO( m_flNextEffectRem ) ),
	RecvPropUtlVectorDataTable( m_activeEffects, MAX_ACTIVE_EFFECTS, DT_ChaosEffect ),
END_RECV_TABLE()


LINK_ENTITY_TO_CLASS( chaos_controller, C_ChaosControllerProxy );
IMPLEMENT_CLIENTCLASS( C_ChaosControllerProxy, DT_ChaosControllerProxy, CChaosControllerProxy );


void RecvProxy_ChaosController( const RecvProp *pProp, void **pOut, void *pData, int objectID )
{
	*pOut = &g_CC;
}

BEGIN_RECV_TABLE( C_ChaosControllerProxy, DT_ChaosControllerProxy )
	RecvPropDataTable( "chaos_controller_data", 0, 0, &REFERENCE_RECV_TABLE( DT_ChaosController ), RecvProxy_ChaosController )
END_RECV_TABLE()


C_ChaosControllerProxy* C_ChaosControllerProxy::s_pChaosControllerProxy = nullptr;
void C_ChaosControllerProxy::NotifyNetworkStateChanged()
{
	if ( s_pChaosControllerProxy )
		s_pChaosControllerProxy->NetworkStateChanged();
}

int C_ChaosControllerProxy::UpdateTransmitState()
{
	return 0;
}


CON_COMMAND(chaos_network_test, "print clientside variables")
{
	Msg("timer: %.2f\n", g_CC.m_flNextEffectRem);
	for (int i = 0; i < g_CC.m_activeEffects.Count(); ++i)
	{
		Msg("%s: %.2f\n", g_CC.m_activeEffects[i].m_strName, g_CC.m_activeEffects[i].m_flTimeRem);
		// Msg("%d: %d\n", i, g_CC.m_activeEffects[i]);
	}
}
