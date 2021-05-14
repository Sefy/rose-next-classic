#pragma once

#define PATCH_COUNT_PER_MAP_AXIS 16

class CQuadPatchManager;
class CMAP_PATCH;
class CMAP;


class CPatchManager {
private:
    CMAP_PATCH* m_ppPATCH[3 * PATCH_COUNT_PER_MAP_AXIS][3 * PATCH_COUNT_PER_MAP_AXIS]; // 맵 3장분

    CMAP_PATCH* m_ppSubPATCH[PATCH_COUNT_PER_MAP_AXIS * PATCH_COUNT_PER_MAP_AXIS * 9 * 2]; //
    short m_nSubPATCH;

    CMAP_PATCH* m_CameraPATCH[PATCH_COUNT_PER_MAP_AXIS * PATCH_COUNT_PER_MAP_AXIS * 10];
    short m_nCameraPatch;

    CMAP_PATCH* m_PickPATCH[PATCH_COUNT_PER_MAP_AXIS * PATCH_COUNT_PER_MAP_AXIS * 9 * 2];
    short m_nPickPATCH;

    WORD m_wViewFRAME;
    float m_ViewFrsutumEq[6][4];

    std::vector<CMAP_PATCH*> patches;
    int patch_lod[31][31];

public:
    CPatchManager(void);
    ~CPatchManager(void);

    CQuadPatchManager* m_ppQuadPatchManager[3][3];
    bool m_isUse[3][3];
    int m_nDrawingPatch;
    bool lod_onoff;

    void ResetViewFrame() { m_wViewFRAME = 0; }
    void IncreaseViewFrame() {
        m_wViewFRAME++;
        if (m_wViewFRAME <= 0)
            m_wViewFRAME = 0;
    }

    void SetPATCH(int iX, int iY, CMAP_PATCH* pPATCH);

    void Update_VisiblePatch(short nMappingX, short nMappingY);
    void Insert_VisiblePatch(CMAP_PATCH* pPATCH);
    void Clear_VisiblePatch(void);
    void Delete_UnvisiblePatch(void);

    float Pick_POSITION(D3DXVECTOR3& PosPICK);
    void ClearAllQuadPatchManager();
    void AddQuadPatchManager(CMAP* pMap, short y, short x);
    void CalculateViewFrustumCulling();
    void AddVisiblePatch(CQuadPatchManager* pQuadPatch);
    void Update_VisiblePatchManager();
    void SetPatchType();
    void ExecutePatchTpye();
    void InsertCameraPatch(CMAP_PATCH* pPatch);
    void TestDrawAABB();
    void AddPickPatch(CQuadPatchManager* pQuadPatch);
    void CalculatePickingPATCH();

    /// for mob collision test
    void DrawCollisionCylinder();
};
