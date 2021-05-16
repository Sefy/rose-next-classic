#include "stdafx.h"

#include ".\citstatenormal.h"
#include "../CDragNDropMgr.h"
#include "../Dlgs/ChattingDLG.h"
#include "../Dlgs/CMinimapDLG.h"
#include "../Dlgs/quicktoolbar.h"

#include "../Icon/CIconDialog.h"
#include "../../System/CGame.h"

#include "tgamectrl/teditbox.h"
#include "tgamectrl/tcontrolmgr.h"

CITStateNormal::CITStateNormal(void) {
    m_iID = IT_MGR::STATE_NORMAL;
}

CITStateNormal::~CITStateNormal(void) {}

void
CITStateNormal::Enter() {}

unsigned
CITStateNormal::Process(unsigned uiMsg, WPARAM wParam, LPARAM lParam) {
    UINT uiRet = 0;

    /// Ctrl들의 내부처리를 위한 Setting
    if (uiMsg == WM_LBUTTONUP)
        CWinCtrl::SetMouseExclusiveCtrl(NULL);

    list_dlgs_ritor ritorDlgs;
    CTDialog* pDlg = NULL;
    int iProcessDialogType = 0;

    /// 일반적인 다이얼로그 처리
    for (ritorDlgs = g_itMGR.m_Dlgs.rbegin(); ritorDlgs != g_itMGR.m_Dlgs.rend(); ++ritorDlgs) {
        pDlg = *ritorDlgs;
        if (pDlg->Process(uiMsg, wParam, lParam)) {
            if (uiMsg == WM_LBUTTONDOWN)
                g_itMGR.MoveDlg2ListEnd(pDlg); /// iterator가 파괴될수 있다 항상 loop를 벗어날것

            uiRet = uiMsg;
            iProcessDialogType = pDlg->GetDialogType();
            break;
        }

        ///모달 다이얼로그일 경우는 다음 다이얼로그를 처리할필요가 없다.
        if (pDlg->IsVision() && pDlg->IsModal()) {
            DWORD dwDialgType = pDlg->GetDialogType();
            return 1;
        }
    }

    /// 아이콘화된 다이얼로그 처리
    std::list<CIconDialog*>::reverse_iterator riterIcons;
    for (riterIcons = g_itMGR.m_Icons.rbegin(); riterIcons != g_itMGR.m_Icons.rend();
         ++riterIcons) {
        if ((*riterIcons)->Process(uiMsg, wParam, lParam)) {
            if (uiMsg == WM_LBUTTONDOWN)
                g_itMGR.MoveDlgIcon2ListEnd(*riterIcons);

            uiRet = uiMsg;
            break;
        }
    }
    /// 메뉴가 오픈된 상태에서 다른곳을 클릭시 메뉴를 닫는다.
    if (uiMsg == WM_LBUTTONDOWN) {
        if (iProcessDialogType != DLG_TYPE_MENU && iProcessDialogType != DLG_TYPE_NOTIFY) {
#ifdef _NEWUI
#else
            g_itMGR.CloseDialog(DLG_TYPE_MENU);
#endif
        }
    }
    
    // 단축키 처리
    if (!this->ProcessHotKey(uiMsg, wParam, lParam)) {
        return 0;
    }

    switch (uiMsg) {
        case WM_KEYDOWN:
            // quick 슬롯처리
            uiRet = uiMsg;
            switch (wParam) {
                case 91: /// Window Key
                     g_itMGR.OpenDialog( DLG_TYPE_MENU );
                    /// 윈도우 시작메뉴가 시작되면서 화면전환이 되버린다. 화면이 떠 있을경우에는
                    /// 후킹해서 막아버릴까? 일단 보류
                    break;

                case VK_ESCAPE: {
                    g_itMGR.ProcessEscKeyDown();
                    break;
                }
                default:
                    uiRet = 0;
                    break;
            }
            break;
        case WM_LBUTTONUP:
            // 드래그앤드롭 처리
            //		g_DragNDrop.DropToWindow();
            CDragNDropMgr::GetInstance().DragEnd(iProcessDialogType);
            g_itMGR.DelDialogiconFromMenu();
            break;
    }

    ///교환중일경우 이동을 막는다.
    if ((pDlg = g_itMGR.FindDlg(DLG_TYPE_EXCHANGE)) && pDlg->IsVision())
        return uiMsg;

    return uiRet;
}

//*-----------------------------------------------------------------------------------------//
/// @brief 채팅을 한번 할때마다 Enter 를 입력 해야 하는 방식
//*-----------------------------------------------------------------------------------------//
bool
CITStateNormal::ProcessHotKey(unsigned uiMsg, WPARAM wParam, LPARAM lParam) {

    // Debug !
    wchar_t msg[32];
    switch (uiMsg) {
        case WM_SYSKEYDOWN:
            swprintf_s(msg, L"WM_SYSKEYDOWN: 0x%x\n", wParam);
            OutputDebugStringW(msg);
            break;

        case WM_SYSCHAR:
            swprintf_s(msg, L"WM_SYSCHAR: %c\n", (wchar_t)wParam);
            OutputDebugStringW(msg);
            break;

        case WM_SYSKEYUP:
            swprintf_s(msg, L"WM_SYSKEYUP: 0x%x\n", wParam);
            OutputDebugStringW(msg);
            break;

        case WM_KEYDOWN:
            swprintf_s(msg, L"WM_KEYDOWN: 0x%x\n", wParam);
            OutputDebugStringW(msg);
            break;

        case WM_KEYUP:
            swprintf_s(msg, L"WM_KEYUP: 0x%x\n", wParam);
            OutputDebugStringW(msg);
            break;

        case WM_CHAR:
            swprintf_s(msg, L"WM_CHAR: %c\n", (wchar_t)wParam);
            OutputDebugStringW(msg);
            break;
    }

    if (uiMsg == WM_SYSKEYDOWN
        || (uiMsg == WM_KEYDOWN && it_GetKeyboardInputType() == CTControlMgr::INPUTTYPE_NORMAL
            && CTEditBox::s_pFocusEdit == NULL)) {

        switch (wParam) {
            // 1 - 4
            case 0x31:
            case 0x32:
            case 0x33:
            case 0x34:
            {
                CQuickBAR* quickbar = (CQuickBAR*)g_itMGR.FindDlg(DLG_TYPE_QUICKBAR);

                if (quickbar) {
                    quickbar->setCurrentPage(wParam - 0x31);
                }

                return true;
            }
            case 0x41: // 'a'
                //캐릭터창을 연다
                g_itMGR.OpenDialog(DLG_TYPE_CHAR);
                return true;
            case 0x43: // 'c'
                g_itMGR.OpenDialog(DLG_TYPE_COMMUNITY);
                return true;
            case 0x48: // h
                g_itMGR.OpenDialog(DLG_TYPE_HELP);
                return true;
            case 0x49: // 'i'
            case 0x56: // 'v' /// 2004 / 1 / 26 / Navy /추가( SYSTEM + I가 한손으로 누르기
                    // 힘들다는 의견으로 )
                    //인벤토리를 연다
                g_itMGR.OpenDialog(DLG_TYPE_ITEM);
                return true;
            ///스킬창
            case 0x53: // 's'
                g_itMGR.OpenDialog(DLG_TYPE_SKILL);
                return true;
            /// 퀘스트창
            case 0x51: // 'q'
                g_itMGR.OpenDialog(DLG_TYPE_QUEST);
                return true;
            ///미니맵 보이기 / 숨기기
            case 0x4d: // 'm'
            {
                CMinimapDLG* pDlg = (CMinimapDLG*)g_itMGR.FindDlg(DLG_TYPE_MINIMAP);
                pDlg->ToggleShowMinimap();
                return true;
            }
            ///미니맵 확대 / 축소
            case 0x4c: // 'l'
            {
                CMinimapDLG* pDlg = (CMinimapDLG*)g_itMGR.FindDlg(DLG_TYPE_MINIMAP);
                pDlg->ToggleZoomMinimap();
                return true;
            }
            case 0x4e: // n
            {
                g_itMGR.OpenDialog(DLG_TYPE_CLAN);
                return true;
            }
            case 0x58: //'x'
                g_itMGR.OpenDialog(DLG_TYPE_SYSTEM);
                return true;
            case 0x4f: //'o'
                g_itMGR.OpenDialog(DLG_TYPE_OPTION);
                return true;
            // case VK_OEM_3: // '~'
            case 0x5A: // 'z'
                g_itMGR.OpenDialog(DLG_TYPE_CONSOLE);
                return true;
            default:
                break;
        }

        return false;
    }
    
    return true;
}
