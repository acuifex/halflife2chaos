#include "c_chaos.h"

C_ChaosController g_CC;

BEGIN_RECV_TABLE_NOBASE( C_ChaosController, DT_ChaosController )
	RecvPropFloat( RECVINFO( m_flNextEffectRem ) ),
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


CON_COMMAND(chaos_network_test, "print clientside timer")
{
	Msg("%.2f\n", g_CC.m_flNextEffectRem);
}
