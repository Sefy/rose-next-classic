#include "stdafx.h"

#include ".\targetmanager.h"
#include "../CViewMSG.h"

#include "../Interface/CTDrawImpl.h"
#include "../Interface/IO_ImageRes.h"
#include "../OBJECT.h"

#include "../JCommandState.h"
#include "tgamectrl/resourcemgr.h"
#include "../Interface/CNameBox.h"

CTargetManager _TargetManager;

CTargetManager::CTargetManager(void) {}

CTargetManager::~CTargetManager(void) {}

void
CTargetManager::SetMouseTargetObject(int iObjectIDX) {
    m_iCurrentMouseTargetObject = iObjectIDX;
}

void
CTargetManager::Proc() {
    if (m_iCurrentMouseTargetObject) {
        g_pViewMSG->AddObjIDX(m_iCurrentMouseTargetObject);
    }

    /// 현재 선택된 오브젝트가 있고
    if (g_UserInputSystem.GetCurrentTarget()) {
        /// 그것이 몹이라면
        CObjCHAR* pObj =
            (CObjCHAR*)g_pObjMGR->Get_CharOBJ(g_UserInputSystem.GetCurrentTarget(), true);
        /// 유효하지 않은 타겟이다.. 마우스 컴맨드 초기화
        if (pObj != NULL) {
            if (pObj->IsA(OBJ_MOB)) {
                g_pViewMSG->AddObjIDX(g_UserInputSystem.GetCurrentTarget());
            }
        }
    }
}

void
CTargetManager::Draw() {

}