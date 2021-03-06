#ifndef __SHO_WS_LIB_H
#define __SHO_WS_LIB_H

#include "util/classstr.h"
#include "classtime.h"
#include "rose/common/server_config.h"

namespace Rose {
namespace Common {
struct DatabaseConfig;
}
} // namespace Rose

class STBDATA;
class SHO_WS {
private:
    static SHO_WS* m_pInstance;

    SHO_WS();
    virtual ~SHO_WS();

    void SystemINIT(HINSTANCE hInstance, char* szBaseDataDIR, int iLangType);

    bool Load_BasicDATA();
    void Free_BasicDATA();
    HWND m_hMainWND;

    int m_iClientListenPortNO;
    int m_iServerListenPortNO;

    CTimer* m_pWorldTIMER;

    friend VOID CALLBACK WS_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    CStrVAR m_BaseDataDIR;
    int m_iLangTYPE;

    CStrVAR m_ServerNAME;
    CStrVAR m_ServerIP;
    int m_iListenPortNO;
    bool m_bBlockCreateCHAR;

    DWORD m_dwRandomSEED;

public:
    bool connect_database(Rose::Common::ServerConfig& config);

    bool Start(HWND hMainWND,
        char* szLoginServerIP,
        int iLoginServerPort,
        char* szLogServerIP,
        int iLogServerPortNO,
        char* szWorldName,
        int iZoneServerListenPort,
        int iGameServerListenPort,
        bool bBlockCreateCHAR);
    void Active(bool bActive);
    void Shutdown();

    bool ConnectToLSV();
    void DisconnectFromLSV();

    bool ConnectToLOG();
    void DisconnectFromLOG();

    char* GetServerName() { return m_ServerNAME.Get(); }
    char* GetServerIP() { return m_ServerIP.Get(); }
    int GetListenPort() { return m_iClientListenPortNO; }
    DWORD GetRandomSeed() { return m_dwRandomSEED; }
    bool IsBlockedCreateCHAR() { return m_bBlockCreateCHAR; }

    void Send_ANNOUNCE(char* szMsg);

    void StartCLI_SOCKET();
    void ShutdownCLI_SOCKET();

    void ToggleChannelActive(int iChannelNo);
    void DeleteChannelServer(int iChannelNo);

    static SHO_WS* GetInstance() { return m_pInstance; }
    static SHO_WS* InitInstance(HINSTANCE hInstance, char* szBaseDataDIR, int iLangType) {
        if (NULL == m_pInstance) {
            // m_pInstance = (class SHO_WS *)1;
            m_pInstance = new SHO_WS();
            _ASSERT(m_pInstance);
            m_pInstance->SystemINIT(hInstance, szBaseDataDIR, iLangType);
        }
        return m_pInstance;
    }
    void Destroy() { SAFE_DELETE(m_pInstance); }

    int GetLangTYPE() { return m_iLangTYPE; }
};

#define WS_TIMER_LSV 1
#define WS_TIMER_LOG 2
#define WS_TIMER_WORLD_TIME 3

#define RECONNECT_TIME_TICK 10000 // 10 sec
#define WORLD_TIME_TICK 10000 // 10 sec

extern VOID CALLBACK WS_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
extern DWORD GetServerBuildNO();
extern DWORD GetServerStartTIME();

inline int
Get_ServerLangTYPE() {
    return SHO_WS::GetInstance()->GetLangTYPE();
}

//-------------------------------------------------------------------------------------------------
#endif