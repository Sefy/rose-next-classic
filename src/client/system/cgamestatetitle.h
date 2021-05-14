#pragma once

#include "cgamestate.h"

/**
 * 클라이언트 실행후 로그인 상태 이전, 타이틀을 보여주며 데이타를 로딩하는 State Class
 *
 * @Author	최종진
 * @Date		2005/9/15
 */
class CGameStateTitle: public CGameState {
public:
public:
    CGameStateTitle(int iID);
    ~CGameStateTitle(void);

    CGameStateTitle() = delete;
    CGameStateTitle(CGameStateTitle&) = delete;
    CGameStateTitle(CGameStateTitle&&) = delete;
    CGameStateTitle& CGameStateTitle::operator=(const CGameStateTitle&) = delete;
    CGameStateTitle& CGameStateTitle::operator=(CGameStateTitle&&) = delete;

    // -- CGameState
    int Update(bool bLostFocus) override;
    int Enter(int iPrevStateID) override;
    int Leave(int iNextStateID) override;
    int ProcMouseInput(UINT uiMsg, WPARAM wParam, LPARAM lParam) override { return 0; }
    int ProcKeyboardInput(UINT uiMsg, WPARAM wParam, LPARAM lParam) override { return 0; }
    // --

    // Handlers
    void ServerDisconnected() {}

protected:
    void Draw();

private:
    std::atomic<bool> data_loaded;
    std::thread data_thread;

    HNODE title_texture;
    int background_zone_id;

private:
    void load_data();
};
