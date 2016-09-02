#pragma once

#include "STXSmoothStopValue.h"
#include <Gdiplus.h>
#include <memory>
#include <queue>

using namespace std;

//////////////////////////////////////////////////////////////////////////

class CSTXListControlExItem;

//////////////////////////////////////////////////////////////////////////
// CSTXListControlExButton

class CSTXListControlExButton
{
	friend class CSTXListControlExItem;
	friend class CSTXListControlEx;
public:
	CSTXListControlExButton(CSTXListControlExItem *pParentItem, IStream *pStreamImage, LPCTSTR lpszToolTips);
	virtual ~CSTXListControlExButton();

protected:
	CString m_strToolTips;		//Not implemented yet
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgImage;
	CSTXListControlExItem *m_pParentItem;

	CSTXSmoothStopValue m_iOffset;
};

//////////////////////////////////////////////////////////////////////////
// CSTXListControlExItem

class CSTXListControlExItem : public CSTXSmoothStopValueEventHandler
{
	friend class CSTXListControlEx;

protected:
	CSTXListControlExItem(CSTXListControlEx *pParentAnimationList, CSTXListControlExItem *pParentItem, DOUBLE fInitialOffset);

public:
	virtual ~CSTXListControlExItem();

protected:
	CSTXSmoothStopValue m_iLeftPadding;		//0-
	CSTXSmoothStopValue m_iOffset;			//0-		//Global or Local offset
	CSTXSmoothStopValue m_vChildItemSpacing;	//0- 
	CSTXSmoothStopValue m_iTextOpacity;		//0.0-1.0
	CSTXSmoothStopValue m_iItemOpacity;		//0.0-1.0
	CSTXSmoothStopValue m_iContentOpacity;	//0.0-1.0

	std::tr1::shared_ptr<Gdiplus::Image> m_pImgImage;
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgImageCollapse;

	CSTXSmoothStopValue m_iImageOpacity;			//0.0-1.0
	CSTXSmoothStopValue m_iHoverMarkOpacity;		//0.0-1.0

	CSTXSmoothStopValue m_vItemBackColorRed;			//0.0 - 1.0
	CSTXSmoothStopValue m_vItemBackColorGreen;			//0.0 - 1.0
	CSTXSmoothStopValue m_vItemBackColorBlue;			//0.0 - 1.0

	COLORREF m_clrMainCaptionColor;
	COLORREF m_clrSubCaptionColor;

protected:
	CString m_strCaption;
	DWORD_PTR m_dwItemData;
	BOOL m_bExpand;
	BOOL m_bExcludeFromFindSubItem;

protected:
	CSTXListControlEx *m_pParentAnimationList;
	CSTXListControlExItem *m_pParentItem;
	CArray <std::tr1::shared_ptr<CSTXListControlExItem> > m_arrChildItems;
	CArray <std::tr1::shared_ptr<CSTXListControlExButton> > m_arrButtons;

	std::queue<std::tr1::shared_ptr<CSTXListControlExItem> > m_queueDeleted;

protected:
	BOOL m_bUseGrayImage;
	CString m_strTag1;
	CString m_strTag2;
	int m_iMarkImage;		// Busy, Away ...
	CString m_strSubCaption;

protected:
	std::tr1::shared_ptr<CSTXListControlExButton> GetButton(INT_PTR iButtonIndex);
	HRESULT DrawItemEx(Gdiplus::Graphics *pGraphics, int iOffsetX, int iOffsetY, int iHeight, int nControlHeight);

//	HRESULT DrawToInternalImage(ID2D1HwndRenderTarget *pHwndRenderTarget);
//	void ClearInternalImage();

	void UpdateVariableTime();

	virtual void ActivateAnimation();
	virtual void DeactivateAnimation();

public:
	int GetHeight();		//Height, including this and children
	int GetHeaderHeight();		//this Header
	INT_PTR InsertNewSubItem( INT_PTR iIndex, LPCTSTR lpszCaption = NULL );

	INT_PTR DeleteSubItem(INT_PTR iSubItemIndex, BOOL bFadeOut);
	void GetSubItemRect(INT_PTR iSubItemIndex, LPRECT lpRect);

	HRESULT DrawChildren(Gdiplus::Graphics *pGraphics, const CRect &rcItem, int nControlHeight);
	void UseGrayImage(BOOL bGray);

	INT_PTR InsertNewSubItemCopy( INT_PTR iIndex, std::tr1::shared_ptr<CSTXListControlExItem> pItem );
	INT_PTR GetSubItemCount();

	CString GetCaption(BOOL bSubCaption = FALSE);
	CString GetTag1();
	CString GetTag2();

	BOOL IsRootItem();
	INT_PTR ButtonHitTest(CPoint point);
	static void DrawImage(Gdiplus::Graphics *pGraphics, Gdiplus::Image *pImage, DOUBLE fAlpha, CRect rcRect);
};



//////////////////////////////////////////////////////////////////////////
// // CSTXListControlEx

#define STXALN_ITEMCLICK		1
typedef struct tagNMSTXALITEMCLICK
{
	NMHDR hdr;
	UINT nMouseButton;
	INT_PTR iIndex;
	BOOL bInHeader;
	INT_PTR iSubIndex;
	DWORD_PTR dwItemData;
}NMSTXALITEMCLICK, *LPNMSTXALITEMCLICK;

#define STXALN_ITEMDBLCLICK		2
typedef struct tagNMSTXALITEMDBLCLICK
{
	NMHDR hdr;
	UINT nMouseButton;
	INT_PTR iIndex;
	BOOL bInHeader;
	INT_PTR iSubIndex;
	DWORD_PTR dwItemData;
}NMSTXALITEMDBLCLICK, *LPNMSTXALITEMDBLCLICK;

#define STXALN_CLICK		3
#define STXALN_RCLICK		4
#define STXALN_EMPTYCLICK	5

#define STXALN_BUTTONCLICK	6
typedef struct tagNMSTXALBUTTONCLICK
{
	NMHDR hdr;
	UINT nMouseButton;
	INT_PTR iIndex;
	INT_PTR iSubIndex;
	DWORD_PTR dwItemData;
	INT_PTR iButtonIndex;
}NMSTXALBUTTONCLICK, *LPNMSTXALBUTTONCLICK;


class CSTXListControlEx : public CWnd, public CSTXSmoothStopValueEventHandler
{
	friend class CSTXListControlExItem;

	DECLARE_DYNAMIC(CSTXListControlEx)

public:
	CSTXListControlEx();
	virtual ~CSTXListControlEx();

protected:
	DECLARE_MESSAGE_MAP()

protected:

protected:
	CArray <std::tr1::shared_ptr<CSTXListControlExItem> > m_arrItems;

	CString m_strTextWhenEmpty;
	std::queue<std::tr1::shared_ptr<CSTXListControlExItem> > m_queueDeleted;

protected:
	std::tr1::shared_ptr<CSTXListControlExItem> m_pLastHoverItem;
	std::tr1::shared_ptr<CSTXListControlExItem> m_pLastLMouseDownItem;
	std::tr1::shared_ptr<CSTXListControlExItem> m_pLastRMouseDownItem;
	std::tr1::shared_ptr<CSTXListControlExButton> m_pLastLMouseDownButton;

	std::tr1::shared_ptr<CSTXListControlExItem> m_pFocusedItem;
	INT_PTR m_iFocusItemIndex;
	INT_PTR m_iFocusSubItemIndex;

	BOOL m_bDragging;
	CPoint m_ptLButtonDown;

protected:
	CSTXSmoothStopValue m_vTopSpacing;
	CSTXSmoothStopValue m_vLeftSpacing;
	CSTXSmoothStopValue m_vRightSpacing;
	CSTXSmoothStopValue m_vBottomSpacing;

	CSTXSmoothStopValue m_vImageAndTextSpacing;	//Space between Image area and Text area
	CSTXSmoothStopValue m_vItemSpacing;			//Space between each item

	std::tr1::shared_ptr<Gdiplus::Image> m_pImgBackground;
	std::tr1::shared_ptr<Gdiplus::CachedBitmap> m_pImgBackgroundCached;

	CSTXSmoothStopValue m_vBackColorRed;			//0.0 - 1.0
	CSTXSmoothStopValue m_vBackColorGreen;			//0.0 - 1.0
	CSTXSmoothStopValue m_vBackColorBlue;			//0.0 - 1.0

	CSTXSmoothStopValue m_vSubItemLeftSpacing;		//0 -

protected:
	CArray<std::tr1::shared_ptr<Gdiplus::Image> > m_arrSubMarkImages;

	std::tr1::shared_ptr<Gdiplus::Image> m_pImgBusy;
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgAway;

	BOOL m_bGlobalUseGrayImage;

protected:
	void DrawControl(CDC *pDC = NULL);
	int CalculateTotalHeight();
	void ResetScrollBars();
	void SetFocusedItem(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	void FocusNextItem();
	void FocusPreviousItem();
	void InternalEnsureVisible(tr1::shared_ptr<CSTXListControlExItem> pItemEx);

	virtual void ActivateAnimation();
	virtual void DeactivateAnimation();

public:
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(IStream *pStream, int nWidthHeight);
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(HBITMAP hBitmap, int nWidthHeight);
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(LPCTSTR lpszFile, int nWidthHeight);
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(std::tr1::shared_ptr<Gdiplus::Image> pImage, int nWidthHeight);


private:
	std::tr1::shared_ptr<CSTXListControlExItem> DeleteSubItemGetCopy(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	INT_PTR InsertNewSubItemCopy(INT_PTR iItemIndex, INT_PTR iSubItemIndex, std::tr1::shared_ptr<CSTXListControlExItem> pItem);

public:
	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	INT_PTR InsertNewItem(INT_PTR iItemIndex, LPCTSTR lpszCaption = NULL);
	INT_PTR InsertNewSubItem(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszCaption = NULL);
	INT_PTR DeleteItem( INT_PTR iItemIndex );
	INT_PTR DeleteSubItem(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	INT_PTR MoveSubItemToEnd(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	INT_PTR MoveSubItemToBegin(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	INT_PTR GetItemCount() const;
	DWORD_PTR GetItemData(INT_PTR iItemIndex);
	INT_PTR SetItemData(INT_PTR iItemIndex, DWORD_PTR dwItemData);
	INT_PTR SetItemCaption(INT_PTR iItemIndex, LPCTSTR lpszCaption, BOOL bSubCaption = FALSE);
	int GetItemWidth();
	int GetTotalHeight();
	INT_PTR ItemHitTest(CPoint point, BOOL *pbHeaderOnly = NULL);
	INT_PTR SubItemHitTest(INT_PTR iItemIndex, CPoint point);
	CString GetSubItemCaption(INT_PTR iItemIndex, INT_PTR iSubItemIndex, BOOL bSubCaption = FALSE);
	INT_PTR SetSubItemCaption(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszCaption, BOOL bSubCaption = FALSE);
	CString GetItemCaption(INT_PTR iItemIndex, BOOL bSubCaption = FALSE);
	void ExpandItem(INT_PTR iItemIndex, BOOL bExpand);
	BOOL IsItemExpanded(INT_PTR iItemIndex);
	HRESULT SetItemImage(INT_PTR iItemIndex, LPCTSTR lpszFile, BOOL bExpanded = TRUE);
	HRESULT SetItemImage(INT_PTR iItemIndex, HBITMAP hBitmap, BOOL bExpanded = TRUE);
	HRESULT SetItemImage(INT_PTR iItemIndex, IStream *pStream, BOOL bExpanded = TRUE);
	HBITMAP CloneBitmap( HBITMAP hSourceBitmap );
	HRESULT SetSubItemImage(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszFile);
	HRESULT SetSubItemImage(INT_PTR iItemIndex, INT_PTR iSubItemIndex, HBITMAP hBitmap);
	HRESULT SetSubItemImage(INT_PTR iItemIndex, INT_PTR iSubItemIndex, IStream *pStream);
	void DeleteAllItems();
	HRESULT SetBackgroundImage(LPCTSTR lpszFile);
	HRESULT SetBackgroundImage(HBITMAP hBitmap);
	HRESULT SetBackgroundImage(IStream *pStream);
	INT_PTR SetSubItemData(INT_PTR iItemIndex, INT_PTR iSubItemIndex, DWORD_PTR dwItemData);
	DWORD_PTR GetSubItemData(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	void SetBackColor( COLORREF clrColor );
	INT_PTR GetSubItemCount(INT_PTR iItemIndex);
	COLORREF GetBackgroundColor();
	INT_PTR SetSubItemImageGray(INT_PTR iItemIndex, INT_PTR iSubItemIndex, BOOL bGray);
	INT_PTR SetSubItemTag1(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszTag);
	INT_PTR SetSubItemTag2(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszTag);
	CString GetSubItemTag1(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	CString GetSubItemTag2(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	INT_PTR FindSubItemWithTag1(INT_PTR *pItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive = FALSE);
	INT_PTR FindSubItemWithTag2(INT_PTR *pItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive = FALSE);
	INT_PTR FindSubItemWithTag1(INT_PTR iItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive = FALSE);
	INT_PTR FindSubItemWithTag2(INT_PTR iItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive = FALSE);
	INT_PTR ClearSubItemMark(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	void SetGlobalUseGrayImage(BOOL bGray);
	void SetEmptyText(LPCTSTR lpszText);
	INT_PTR SetItemTag1(INT_PTR iItemIndex, LPCTSTR lpszTag);
	INT_PTR FindItemWithTag1(LPCTSTR lpszTag, BOOL bCaseSensitive = FALSE);
	INT_PTR DeleteAllSubItems( INT_PTR iItemIndex );
	INT_PTR SetItemCaptionColor(INT_PTR iItemIndex, COLORREF clrColor);
	INT_PTR SetSubItemCaptionColor(INT_PTR iItemIndex, INT_PTR iSubItemIndex, COLORREF clrColor);
	INT_PTR SetSubItemSubCaptionColor( INT_PTR iItemIndex, INT_PTR iSubItemIndex, COLORREF clrColor );
	INT_PTR ExcludeItemFromFindSubItem(INT_PTR iItemIndex, BOOL bExclude);

	INT_PTR SetSubImage(INT_PTR iImageIndex, IStream *pStream);
	INT_PTR SetSubImage(INT_PTR iImageIndex, HBITMAP hBitmap);
	INT_PTR SetSubImage(INT_PTR iImageIndex, LPCTSTR lpszFile);
	INT_PTR SetSubItemSubImageIndex(INT_PTR iItemIndex, INT_PTR iSubItemIndex, INT_PTR iImageIndex);

	INT_PTR AddItemButton(INT_PTR iItemIndex, IStream *pStreamImage, LPCTSTR lpszToolTips);
	INT_PTR AddSubItemButton(INT_PTR iItemIndex, INT_PTR iSubItemIndex, IStream *pStreamImage, LPCTSTR lpszToolTips);
	CString GetItemTag1(INT_PTR iItemIndex);
	CString GetItemTag2(INT_PTR iItemIndex);
	INT_PTR GetSubItemButtonCount(INT_PTR iItemIndex, INT_PTR iSubItemIndex);
	INT_PTR SetCurSel( INT_PTR iItemIndex, INT_PTR iSubItemIndex );
	void SetSubItemLeftPadding( int nPadding );

protected:
	virtual void OnBeginDrag(std::tr1::shared_ptr<CSTXListControlExItem> pDragItem);
	virtual UINT GetTopReservedHeight();
	virtual void DrawTopReservedSpace(Gdiplus::Graphics *pGraphics);
	virtual void DrawBackground(Gdiplus::Graphics *pGraphics);


public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnUpKeyDown();
	void OnDownKeyDown();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnSpaceKeyDown();
	void OnLeftKeyDown();
	void OnRightKeyDown();
};


