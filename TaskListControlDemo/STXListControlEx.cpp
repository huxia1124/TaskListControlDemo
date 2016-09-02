// STXListControlEx.cpp : 实现文件
//

#include "stdafx.h"
#include "STXListControlEx.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////
#define DEFAULT_ITEM_HEIGHT		18
#define DEFAULT_HEADER_HEIGHT	20
#define MAX_SUBIMAGE_WIDTH		16

#define DEFAULT_BUTTON_WIDTH	16

#define ITEM_ANIMATION_DURATION		0.4

//////////////////////////////////////////////////////////////////////////
// CSTXListControlExButton

CSTXListControlExButton::CSTXListControlExButton(CSTXListControlExItem *pParentItem, IStream *pStreamImage, LPCTSTR lpszToolTips)
: m_iOffset(0, ITEM_ANIMATION_DURATION, pParentItem)
{
	m_strToolTips = lpszToolTips;
	m_pImgImage = CSTXListControlEx::GetResizedImage(pStreamImage, DEFAULT_ITEM_HEIGHT);
}

CSTXListControlExButton::~CSTXListControlExButton()
{

}

//////////////////////////////////////////////////////////////////////////
// CSTXListControlExItem

CSTXListControlExItem::CSTXListControlExItem(CSTXListControlEx *pParentAnimationList, CSTXListControlExItem *pParentItem, DOUBLE fInitialOffset)
: m_iItemOpacity(0, ITEM_ANIMATION_DURATION, this)
, m_dwItemData(0)
	,m_iTextOpacity(1.0, 0.1, this)
	,m_iImageOpacity(1.0, 0.1, this)
	,m_iHoverMarkOpacity(0, 0.1, this)
	,m_iContentOpacity(1.0, ITEM_ANIMATION_DURATION, this)
	,m_iOffset(fInitialOffset, ITEM_ANIMATION_DURATION, this)
	,m_vChildItemSpacing(2, ITEM_ANIMATION_DURATION, this)
	,m_iLeftPadding(96, ITEM_ANIMATION_DURATION, this)
	,m_vItemBackColorRed(0, ITEM_ANIMATION_DURATION, this)
	,m_vItemBackColorGreen(0, ITEM_ANIMATION_DURATION, this)
	,m_vItemBackColorBlue(0, ITEM_ANIMATION_DURATION, this)
{
	m_bExpand = TRUE;
	m_bExcludeFromFindSubItem = FALSE;
	m_pParentAnimationList = pParentAnimationList;
	m_pParentItem = pParentItem;

	m_bUseGrayImage = FALSE;
	m_iMarkImage = -1;
	m_clrMainCaptionColor = GetSysColor(COLOR_WINDOWTEXT);
	m_clrSubCaptionColor = RGB(168, 168, 168);
}

CSTXListControlExItem::~CSTXListControlExItem()
{
}

int CSTXListControlExItem::GetHeaderHeight()
{
	return DEFAULT_HEADER_HEIGHT;
}

int CSTXListControlExItem::GetHeight()
{
	if(m_arrChildItems.GetCount() == 0 || !m_bExpand)
		return GetHeaderHeight();

	return (int)(GetHeaderHeight() + m_arrChildItems.GetCount() * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue()) + m_vChildItemSpacing.GetFinalValue());
}

HRESULT CSTXListControlExItem::DrawItemEx(Gdiplus::Graphics *pGraphics, int iOffsetX, int iOffsetY, int iHeight, int nControlHeight)
{
	int fImageHeight = (int)GetHeaderHeight();
	int fItemWidth = (int)m_pParentAnimationList->GetItemWidth();

	CRect rcItem(0, 0, fItemWidth, fImageHeight);
	rcItem.OffsetRect(iOffsetX, iOffsetY);

	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(0, 0, 0, 0));
	pGraphics->FillRectangle(&bkBrush, rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height());

	//Draw Focus Mark
	CRect rectHeaderRectangle(0, 0, fItemWidth, fImageHeight);
	rectHeaderRectangle.OffsetRect(iOffsetX, iOffsetY);
	if(this == m_pParentAnimationList->m_pFocusedItem.get())
	{
		Gdiplus::SolidBrush bkBrushFocus(Gdiplus::Color(255, 198, 226, 247));
		pGraphics->FillRectangle(&bkBrushFocus, rectHeaderRectangle.left, rectHeaderRectangle.top, rectHeaderRectangle.Width(), rectHeaderRectangle.Height());
	}
	else if(this == m_pParentAnimationList->m_pLastHoverItem.get())
	{
		//Draw Hover Mark
		Gdiplus::SolidBrush bkBrushHover(Gdiplus::Color((BYTE)(255 * m_iHoverMarkOpacity.GetValue()), 227, 241, 250));
		pGraphics->FillRectangle(&bkBrushHover, rectHeaderRectangle.left, rectHeaderRectangle.top, rectHeaderRectangle.Width(), rectHeaderRectangle.Height());
	}
// 	else
// 	{
// 		Gdiplus::SolidBrush bkBrushHover(Gdiplus::Color((BYTE)(255 * m_iHoverMarkOpacity.GetValue()), 255, 180, 0));
// 		pGraphics->FillRectangle(&bkBrushHover, rectHeaderRectangle.left, rectHeaderRectangle.top, rectHeaderRectangle.Width(), rectHeaderRectangle.Height());
// 	}

	// Draw The Image Area
	CRect rectImage(0, 0, fImageHeight, fImageHeight);
	rectImage.OffsetRect(iOffsetX, iOffsetY);

	//m_spRenderTarget->FillRectangle(&rectImage, spBrush);
	//D2D1_RECT_F rectItemRectangle = D2D1::RectF(0, 0, fItemWidth, (FLOAT)GetHeight());
	//m_spRenderTarget->DrawRectangle(&rectItemRectangle,spBrush, 0.5f, NULL);

	Gdiplus::REAL fItemAlpha = (float)m_iItemOpacity.GetValue();

	tr1::shared_ptr<Gdiplus::Image> pImageUsing = m_pImgImage;
	if((!m_bExpand) && m_pImgImageCollapse)
		pImageUsing = m_pImgImageCollapse;

	if(pImageUsing)
	{
		Gdiplus::ColorMatrix cm = { 
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, fItemAlpha, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};

		Gdiplus::ImageAttributes ImgAttr;

		ImgAttr.SetColorMatrix(&cm, Gdiplus::ColorMatrixFlagsDefault, 
			Gdiplus::ColorAdjustTypeBitmap);

		UINT nImgWidth = pImageUsing->GetWidth();
		UINT nImgHeight = pImageUsing->GetHeight();
		Gdiplus::RectF rectImg(0, 0, (float)nImgWidth, (float)nImgHeight);

		Gdiplus::TextureBrush brushImg(pImageUsing.get(), rectImg, &ImgAttr);

		brushImg.TranslateTransform((float)rectImage.left, (float)rectImage.top);
		pGraphics->FillRectangle(&brushImg, rectImage.left, rectImage.top, rectImage.Width(), rectImage.Height());
	}
	else
	{
		//pDC->Rectangle(rectImage);
	}

	//Calculate the size of the Buttons
	int iButtonsWidth = m_arrButtons.GetCount() * DEFAULT_BUTTON_WIDTH;

	//Draw The Caption (Text)
	int fTextLeft = (int)(fImageHeight + m_pParentAnimationList->m_vImageAndTextSpacing.GetValue());
	CRect rectText(fTextLeft, 0, fTextLeft + fItemWidth - iButtonsWidth, fImageHeight);
	rectText.OffsetRect(iOffsetX, iOffsetY);

	Gdiplus::FontFamily  fontFamily(L"Times New Roman");
	Gdiplus::Font drawFont(&fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	Gdiplus::SolidBrush textBrush(Gdiplus::Color(fItemAlpha * 255, GetRValue(m_clrMainCaptionColor), GetGValue(m_clrMainCaptionColor), GetBValue(m_clrMainCaptionColor)));

	Gdiplus::PointF ptText(fTextLeft, 0);
	ptText.X += iOffsetX;
	ptText.Y += iOffsetY;

	// Calculate the size of the Main Caption
	Gdiplus::RectF rcMainCaption;
	pGraphics->MeasureString(m_strCaption, -1, &drawFont, ptText, &rcMainCaption);
	if(DEFAULT_HEADER_HEIGHT - rcMainCaption.Height > 0)
		ptText.Y += ((DEFAULT_HEADER_HEIGHT - rcMainCaption.Height) / 2);

	// Draw the Main Caption
	pGraphics->DrawString(m_strCaption, -1, &drawFont, ptText, &textBrush);

	if(!m_strSubCaption.IsEmpty())
	{
		// Draw SubCaption right after it
		ptText.X += rcMainCaption.Width;
		//ptText.Y += 2;
		Gdiplus::Font drawFontSmall(_T("Arial"), 8);
		pGraphics->DrawString(m_strSubCaption, -1, &drawFontSmall, ptText, &textBrush);
	}

	//Draw Buttons
	CRect rcButtonBase = rcItem;
	rcButtonBase.left = rcButtonBase.right - DEFAULT_BUTTON_WIDTH;
	rcButtonBase.OffsetRect(DEFAULT_BUTTON_WIDTH, 0);
	for(INT_PTR i=0;i<m_arrButtons.GetCount();i++)
	{
		CRect rcButton = rcButtonBase;
		std::tr1::shared_ptr<CSTXListControlExButton> pButton = m_arrButtons[i];
		rcButton.OffsetRect((int)pButton->m_iOffset.GetValue(), 0);

		if(pButton->m_pImgImage)
		{
			DrawImage(pGraphics, pButton->m_pImgImage.get(), fItemAlpha, rcButton);
			//pGraphics->DrawImage(pButton->m_pImgImage.get(), rcButton.left, rcButton.top, rcButton.Width(), rcButton.Height());
		}
		else
		{
			Gdiplus::SolidBrush bkBrushFocus(Gdiplus::Color(255, 198, 198, 198));
			pGraphics->FillRectangle(&bkBrushFocus, rcButton.left, rcButton.top, rcButton.Width(), rcButton.Height());
		}
	}

	rcItem.bottom = rcItem.top + iHeight;
	DrawChildren(pGraphics, rcItem, nControlHeight);

	return S_OK;
}

INT_PTR CSTXListControlExItem::InsertNewSubItem( INT_PTR iIndex, LPCTSTR lpszCaption /*= NULL*/ )
{
	if(iIndex < 0 || iIndex > m_arrChildItems.GetCount())
		return -1;

	DOUBLE fOffsetTarget = 0.0;
	DOUBLE fOffsetInit = 0.0;
	DOUBLE fLeftTarget = m_pParentAnimationList->m_vSubItemLeftSpacing.GetValue();
	DOUBLE fLeftInit = 0.0;

	DOUBLE fHeaderHeight = GetHeaderHeight();

	if(iIndex == m_arrChildItems.GetCount())		//Insert at the end (append)
	{
		fOffsetTarget = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + iIndex * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
		fOffsetInit = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + (iIndex + 1) * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
	}
	else	//Insert in the imddle or beginning
	{
		//Calculate the height of existing items before insertion point, including item spaces
		fOffsetTarget = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + iIndex * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
		fOffsetInit = fOffsetTarget - 4;

		//Adjust all the offsets after this item
		for(INT_PTR i=iIndex;i<m_arrChildItems.GetCount();i++)
		{
			CSTXListControlExItem *pItem = m_arrChildItems[i].get();
			pItem->m_iOffset = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + (i + 1) * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
		}
	}

	std::tr1::shared_ptr<CSTXListControlExItem> pNewItem (new CSTXListControlExItem(m_pParentAnimationList, this, fOffsetTarget));

	if(lpszCaption)
		pNewItem->m_strCaption = lpszCaption;
	//pNewItem->m_iOffset.Reinitialize(fOffsetInit);
	//pNewItem->m_iOffset = fOffsetTarget + fHeaderHeight;
	pNewItem->m_iLeftPadding = fLeftTarget;
	pNewItem->m_iItemOpacity = 1.0f;

	m_arrChildItems.InsertAt(iIndex, pNewItem);

	//m_pParentAnimationList->GetAnimationManager()->ScheduleAnimation();
	//ActivateAnimation();

	return iIndex;
}

HRESULT CSTXListControlExItem::DrawChildren(Gdiplus::Graphics *pGraphics, const CRect &rcItem, int nControlHeight)
{
	if(!m_bExpand)
		return S_OK;

	int fHeaderHeight = DEFAULT_ITEM_HEIGHT;
	int fImageHeight = DEFAULT_ITEM_HEIGHT;
	
	CRect rectChildItem(0, 0, rcItem.Width(), DEFAULT_ITEM_HEIGHT);
	rectChildItem.OffsetRect(rcItem.left, rcItem.top);


	std::queue<std::tr1::shared_ptr<CSTXListControlExItem> > m_queueAllToDraw;

	//Draw Deleted items
	size_t nDrawDelete = m_queueDeleted.size();
	for(size_t i=0;i<nDrawDelete;i++)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_queueDeleted.front();
		m_queueDeleted.pop();

 		if(pItem->m_iItemOpacity.GetValue() > 0.02f)
 		{
 			m_queueDeleted.push(pItem);
 			m_queueAllToDraw.push(pItem);
 		}
	}

	for(INT_PTR i=0;i<m_arrChildItems.GetCount();i++)
	{
		m_queueAllToDraw.push(m_arrChildItems[i]);
	}

	size_t nItemToDraw = m_queueAllToDraw.size();

	//Draw Valid Items
	//for(INT_PTR i=0;i<m_arrChildItems.GetCount();i++)
	for(size_t i=0;i<nItemToDraw;i++)
	{
		//tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrChildItems[i];
		tr1::shared_ptr<CSTXListControlExItem> pItem = m_queueAllToDraw.front();
		m_queueAllToDraw.pop();

		// Draw The Image Area

		DOUBLE fItemOffset = pItem->m_iOffset.GetValue();
		CRect rectSubItemRectangle((int)pItem->m_iLeftPadding.GetValue() + rcItem.left, static_cast<int>(floor(fItemOffset + 0.5f)), 0, 0);
		rectSubItemRectangle.right = /*rectSubItemRectangle.left + */rectChildItem.right;
		rectSubItemRectangle.bottom = rectSubItemRectangle.top + DEFAULT_ITEM_HEIGHT;
		rectSubItemRectangle.OffsetRect(0, rcItem.top);

		if(rectSubItemRectangle.bottom < 0)
			continue;
		if(fItemOffset > rcItem.Height())
			continue;
		if(rectSubItemRectangle.top > nControlHeight)
			break;

		CRect rectImage(rectSubItemRectangle.left, rectSubItemRectangle.top, 0, 0);
		rectImage.right = (FLOAT)(rectImage.left + fImageHeight);
		rectImage.bottom = rectImage.top + DEFAULT_ITEM_HEIGHT;

		//Draw Focus Mark
		if(pItem == m_pParentAnimationList->m_pFocusedItem)
		{
			Gdiplus::SolidBrush brushFocus(Gdiplus::Color(255, 198, 226, 247));
			pGraphics->FillRectangle(&brushFocus, rectSubItemRectangle.left, rectSubItemRectangle.top, rectSubItemRectangle.Width(), rectSubItemRectangle.Height());
		}
		else if(pItem == m_pParentAnimationList->m_pLastHoverItem)
		{
			//Draw Hover Mark
			Gdiplus::SolidBrush brushHover(Gdiplus::Color(255 * pItem->m_iHoverMarkOpacity.GetValue(), 227, 241, 250));
			pGraphics->FillRectangle(&brushHover, rectSubItemRectangle.left, rectSubItemRectangle.top, rectSubItemRectangle.Width(), rectSubItemRectangle.Height());
		}

		DOUBLE fItemAlpha = pItem->m_iItemOpacity.GetValue() * m_iContentOpacity.GetValue();
		if(pItem->m_pImgImage)
		{
			Gdiplus::ColorMatrix *pCMUse = NULL;
			Gdiplus::ColorMatrix cm = { 
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, fItemAlpha, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
			};

			pCMUse = &cm;

			Gdiplus::ColorMatrix cmGray = { 
				0.299, 0.299, 0.299, 0.0f, 0.0f,
				0.587, 0.587, 0.587, 0.0f, 0.0f,
				0.114, 0.114, 0.114, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, fItemAlpha * 0.3, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 1.0f
			};

			if(pItem->m_bUseGrayImage || m_pParentAnimationList->m_bGlobalUseGrayImage)
			{
				pCMUse = &cmGray;
			}

			Gdiplus::ImageAttributes ImgAttr;

			ImgAttr.SetColorMatrix(pCMUse, Gdiplus::ColorMatrixFlagsDefault, 
				Gdiplus::ColorAdjustTypeBitmap);

			UINT nImgWidth = pItem->m_pImgImage->GetWidth();
			UINT nImgHeight = pItem->m_pImgImage->GetHeight();
			Gdiplus::RectF rectImg(0, 0, nImgWidth, nImgHeight);

			Gdiplus::TextureBrush brushImg(pItem->m_pImgImage.get(), rectImg, &ImgAttr);

			brushImg.TranslateTransform(rectImage.left, rectImage.top);
			pGraphics->FillRectangle(&brushImg, rectImage.left, rectImage.top, rectImage.Width(), rectImage.Height());

			//Draw MarkImage (only when non-gray)
			if(!(pItem->m_bUseGrayImage || m_pParentAnimationList->m_bGlobalUseGrayImage))
			{
				if(pItem->m_iMarkImage != -1)
				{
					if(m_pParentAnimationList->m_arrSubMarkImages[pItem->m_iMarkImage] != NULL)
					{
						int iSubImageWidth = DEFAULT_ITEM_HEIGHT / 2;
						
						if(iSubImageWidth > MAX_SUBIMAGE_WIDTH)
							iSubImageWidth = MAX_SUBIMAGE_WIDTH;

						pGraphics->DrawImage(m_pParentAnimationList->m_arrSubMarkImages[pItem->m_iMarkImage].get(), rectImage.right - iSubImageWidth, rectImage.bottom - iSubImageWidth, 0, 0, iSubImageWidth, iSubImageWidth, Gdiplus::UnitPixel);
					}
				}

// 				if(pItem->m_iMarkImage == 1)	//Busy
// 				{
// 					pGraphics->DrawImage(m_pParentAnimationList->m_pImgBusy.get(), rectImage.right - 11, rectImage.bottom - 11, 0, 0, 11, 11, Gdiplus::UnitPixel);
// 				}
// 				else if(pItem->m_iMarkImage == 2)	//Away
// 				{
// 					pGraphics->DrawImage(m_pParentAnimationList->m_pImgAway.get(), rectImage.right - 11, rectImage.bottom - 11, 0, 0, 11, 11, Gdiplus::UnitPixel);
// 				}
			}
		}
		else
		{
			//::Rectangle(hDC, rectImage.left, rectImage.top, rectImage.right, rectImage.bottom);
		}

		//Draw The Caption (Text)
		int fTextLeft = (int)(rectImage.right + m_pParentAnimationList->m_vImageAndTextSpacing.GetValue());

		CRect rectText(fTextLeft, 0, fTextLeft + (int)rcItem.right, DEFAULT_ITEM_HEIGHT);

		Gdiplus::Font drawFont(_T("Arial"), 8);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(fItemAlpha * 255, GetRValue(pItem->m_clrMainCaptionColor), GetGValue(pItem->m_clrMainCaptionColor), GetBValue(pItem->m_clrMainCaptionColor)));

		Gdiplus::PointF ptText(fTextLeft, rectSubItemRectangle.top);

		// Calculate the size of the Main Caption
		Gdiplus::RectF rcMainCaption;
		pGraphics->MeasureString(pItem->m_strCaption, -1, &drawFont, ptText, &rcMainCaption);
		if(DEFAULT_ITEM_HEIGHT - rcMainCaption.Height > 0)
			ptText.Y += ((DEFAULT_ITEM_HEIGHT - rcMainCaption.Height) / 2);

		// Draw the Main Caption
		pGraphics->DrawString(pItem->m_strCaption, -1, &drawFont, ptText, &textBrush);

		if(!pItem->m_strSubCaption.IsEmpty())
		{
			// Draw SubCaption right after it
			ptText.X += rcMainCaption.Width;
			Gdiplus::SolidBrush textBrushSubCaption(Gdiplus::Color(fItemAlpha * 255, GetRValue(pItem->m_clrSubCaptionColor), GetGValue(pItem->m_clrSubCaptionColor), GetBValue(pItem->m_clrSubCaptionColor)));
			pGraphics->DrawString(pItem->m_strSubCaption, -1, &drawFont, ptText, &textBrushSubCaption);
		}


		Gdiplus::RectF rectItemF(rcItem.left + rectSubItemRectangle.left, rcItem.top + pItem->m_iOffset.GetValue(), rectChildItem.Width(), rectChildItem.Height());

		//Draw Buttons
		CRect rcButtonBase = rectSubItemRectangle;
		rcButtonBase.left = rcButtonBase.right - DEFAULT_BUTTON_WIDTH;
		rcButtonBase.OffsetRect(DEFAULT_BUTTON_WIDTH, 0);
		for(INT_PTR i=0;i<pItem->m_arrButtons.GetCount();i++)
		{
			CRect rcButton = rcButtonBase;
			std::tr1::shared_ptr<CSTXListControlExButton> pButton = pItem->m_arrButtons[i];
			rcButton.OffsetRect((int)pButton->m_iOffset.GetValue(), 0);

			if(pButton->m_pImgImage)
			{
				DrawImage(pGraphics, pButton->m_pImgImage.get(), fItemAlpha, rcButton);
				//pGraphics->DrawImage(pButton->m_pImgImage.get(), rcButton.left, rcButton.top, rcButton.Width(), rcButton.Height());
			}
			else
			{
				Gdiplus::SolidBrush bkBrushFocus(Gdiplus::Color(255, 198, 198, 198));
				pGraphics->FillRectangle(&bkBrushFocus, rcButton.left, rcButton.top, rcButton.Width(), rcButton.Height());
			}
		}
	}

	return S_OK;
}

INT_PTR CSTXListControlExItem::DeleteSubItem(INT_PTR iSubItemIndex, BOOL bFadeOut)
{
	if(iSubItemIndex < 0 || iSubItemIndex > m_arrChildItems.GetCount() - 1)
		return -1;

	//Calculate the Initialize and Target offset
// 	CRect rcThis;
// 	GetClientRect(rcThis);

// 	DOUBLE fImageHeight = rcThis.Height() - m_vTopSpacing.GetValue() - m_vBottomSpacing.GetValue();
// 	DOUBLE fItemWidth = fImageHeight + m_vImageAndTextSpacing.GetValue() + m_vTextAreaWidth.GetValue();


	//Adjust all the offsets after this item
	for(INT_PTR i=iSubItemIndex + 1;i<m_arrChildItems.GetCount();i++)
	{
		CSTXListControlExItem *pItem = m_arrChildItems[i].get();
		pItem->m_iOffset = GetHeaderHeight() + m_vChildItemSpacing.GetFinalValue() + (i - 1) * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetValue());
	}

	//Important: m_bExpand must be check
	// Because m_queueDeleted will be examed in the DrawChildren method.
	// if not expanded, DrawChildren will not be called and m_queueDeleted will not be updated. This will
	// cause the activation count to remain positive forever.
 	if(bFadeOut && m_bExpand)
 	{
 		std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrChildItems[iSubItemIndex];
 		m_queueDeleted.push(pItem);
 		pItem->m_iItemOpacity = 0.0f;
		pItem->m_iItemOpacity.GetValue();
 	}

	m_arrChildItems.RemoveAt(iSubItemIndex);

	return iSubItemIndex;
}

void CSTXListControlExItem::GetSubItemRect( INT_PTR iSubItemIndex, LPRECT lpRect )
{
	tr1::shared_ptr<CSTXListControlExItem> pSubItem = m_arrChildItems[iSubItemIndex];

	lpRect->top = (int)pSubItem->m_iOffset.GetValue();
	lpRect->left = (int)pSubItem->m_iLeftPadding.GetValue();
	lpRect->right = (int)(lpRect->left + m_pParentAnimationList->GetItemWidth() - pSubItem->m_iLeftPadding.GetValue());
	lpRect->bottom = (int)(lpRect->top + DEFAULT_ITEM_HEIGHT);

}

void CSTXListControlExItem::UpdateVariableTime()
{
	m_iItemOpacity.GetValue();
	m_iTextOpacity.GetValue();
	m_iImageOpacity.GetValue();
	m_iHoverMarkOpacity.GetValue();
	m_iContentOpacity.GetValue();
	m_iOffset.GetValue();
	m_vChildItemSpacing.GetValue();
	m_iLeftPadding.GetValue();
	m_vItemBackColorRed.GetValue();
	m_vItemBackColorGreen.GetValue();
	m_vItemBackColorBlue.GetValue();

	for(INT_PTR i=0;i<m_arrChildItems.GetCount();i++)
	{
		tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrChildItems[i];
		pItem->UpdateVariableTime();
	}

}

void CSTXListControlExItem::UseGrayImage( BOOL bGray )
{
	m_bUseGrayImage = bGray;
}

INT_PTR CSTXListControlExItem::InsertNewSubItemCopy( INT_PTR iIndex, std::tr1::shared_ptr<CSTXListControlExItem> pItem )
{
	if(iIndex < 0 || iIndex > m_arrChildItems.GetCount())
		return -1;

	DOUBLE fOffsetTarget = 0.0;
	DOUBLE fOffsetInit = 0.0;
	DOUBLE fLeftTarget = m_pParentAnimationList->m_vSubItemLeftSpacing.GetValue();
	DOUBLE fLeftInit = 0.0;

	DOUBLE fHeaderHeight = GetHeaderHeight();

	if(iIndex == m_arrChildItems.GetCount())		//Insert at the end (append)
	{
		fOffsetTarget = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + iIndex * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
		fOffsetInit = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + (iIndex + 1) * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
	}
	else	//Insert in the imddle or beginning
	{
		//Calculate the height of existing items before insertion point, including item spaces
		fOffsetTarget = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + iIndex * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
		fOffsetInit = fOffsetTarget - 4;

		//Adjust all the offsets after this item
		for(INT_PTR i=iIndex;i<m_arrChildItems.GetCount();i++)
		{
			CSTXListControlExItem *pItem = m_arrChildItems[i].get();
			pItem->m_iOffset = fHeaderHeight + m_vChildItemSpacing.GetFinalValue() + (i + 1) * (DEFAULT_ITEM_HEIGHT + m_vChildItemSpacing.GetFinalValue());
		}
	}

	std::tr1::shared_ptr<CSTXListControlExItem> pNewItem = pItem;


	//pNewItem->m_iOffset.SetInitialValueAndVelocity(fOffsetInit, 0);

	pNewItem->m_iOffset = fOffsetTarget;
	pNewItem->m_iLeftPadding = fLeftTarget;
	pNewItem->m_iItemOpacity = 1.0f;

	m_arrChildItems.InsertAt(iIndex, pNewItem);

	//m_pParentAnimationList->GetAnimationManager()->ScheduleAnimation();
	//ActivateAnimation();

	return iIndex;
}

void CSTXListControlExItem::ActivateAnimation()
{
	m_pParentAnimationList->IncreaseActivate();
}

void CSTXListControlExItem::DeactivateAnimation()
{
	m_pParentAnimationList->DecreaseActivate();
}

INT_PTR CSTXListControlExItem::GetSubItemCount()
{
	return m_arrChildItems.GetCount();
}

CString CSTXListControlExItem::GetCaption( BOOL bSubCaption /*= FALSE*/ )
{
	if(bSubCaption)
		return m_strSubCaption;

	return m_strCaption;
}

CString CSTXListControlExItem::GetTag1()
{
	return m_strTag1;
}

CString CSTXListControlExItem::GetTag2()
{
	return m_strTag2;
}

BOOL CSTXListControlExItem::IsRootItem()
{
	return (m_pParentItem == NULL);
}

INT_PTR CSTXListControlExItem::ButtonHitTest( CPoint point )
{
	INT_PTR nCount = m_arrButtons.GetCount();
	if(nCount == 0)
		return -1;

	for(INT_PTR i=0;i<nCount;i++)
	{
		std::tr1::shared_ptr<CSTXListControlExButton> pButton = m_arrButtons[i];
		CRect rcButton((int)pButton->m_iOffset.GetValue(), 0, 0, DEFAULT_ITEM_HEIGHT);
		rcButton.right = rcButton.left + DEFAULT_BUTTON_WIDTH;
		rcButton.OffsetRect(m_pParentAnimationList->GetItemWidth(), 0);

		if(rcButton.PtInRect(point))
			return i;
	}

	return -1;
}

std::tr1::shared_ptr<CSTXListControlExButton> CSTXListControlExItem::GetButton( INT_PTR iButtonIndex )
{
	return m_arrButtons[iButtonIndex];
}

void CSTXListControlExItem::DrawImage( Gdiplus::Graphics *pGraphics, Gdiplus::Image *pImage, DOUBLE fAlpha, CRect rcRect )
{
	Gdiplus::ColorMatrix cm = { 
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, fAlpha, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	Gdiplus::ImageAttributes ImgAttr;

	ImgAttr.SetColorMatrix(&cm, Gdiplus::ColorMatrixFlagsDefault, 
		Gdiplus::ColorAdjustTypeBitmap);

	UINT nImgWidth = pImage->GetWidth();
	UINT nImgHeight = pImage->GetHeight();
	Gdiplus::RectF rectImg(0, 0, (float)nImgWidth, (float)nImgHeight);

	Gdiplus::TextureBrush brushImg(pImage, rectImg, &ImgAttr);

	brushImg.TranslateTransform((float)rcRect.left, (float)rcRect.top);
	pGraphics->FillRectangle(&brushImg, rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height());
}


//////////////////////////////////////////////////////////////////////////
// CSTXListControlEx

IMPLEMENT_DYNAMIC(CSTXListControlEx, CWnd)

CSTXListControlEx::CSTXListControlEx()
:m_ptLButtonDown(-1, -1)
,m_bDragging(FALSE)
,m_bGlobalUseGrayImage(FALSE)
,m_vTopSpacing(2, 0.4, this)
,m_vLeftSpacing(2, 0.4, this)
,m_vRightSpacing(2, 0.4, this)
,m_vBottomSpacing(2, 0.4, this)
,m_vBackColorRed(1, 0.4, this)
,m_vBackColorGreen(1, 0.4, this)
,m_vBackColorBlue(1, 0.4, this)
,m_vImageAndTextSpacing(2, 0.4, this)
,m_vItemSpacing(1, 0.4, this)
,m_vSubItemLeftSpacing(16.0, 0.4, this)
{
	//m_clrHoverItemBackColor = RGB(224, 192, 16);

	m_iFocusItemIndex = -1;
	m_iFocusSubItemIndex = -1;
}

CSTXListControlEx::~CSTXListControlEx()
{

}


BEGIN_MESSAGE_MAP(CSTXListControlEx, CWnd)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()



// CSTXListControlEx 消息处理程序


BOOL CSTXListControlEx::Create( LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	LPCTSTR pszCreateClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW));
	BOOL bResult = __super::Create(pszCreateClass,lpszWindowName,dwStyle,rect,pParentWnd,nID);
	if(!bResult)
		return FALSE;

	//ActivateAnimation();
	return TRUE;

}

void CSTXListControlEx::ActivateAnimation()
{
	if(IsWindow(m_hWnd)) 
		SetTimer(1, 5, NULL);
}

void CSTXListControlEx::DeactivateAnimation()
{
	if(IsWindow(m_hWnd)) 
	{
		KillTimer(1);
		Invalidate();
	}
}


void CSTXListControlEx::OnTimer(UINT_PTR nIDEvent)
{
	if(!IsWindowVisible())
		return;

	m_vTopSpacing.GetValue();
	m_vLeftSpacing.GetValue();
	m_vRightSpacing.GetValue();
	m_vBottomSpacing.GetValue();

	m_vImageAndTextSpacing.GetValue();
	m_vItemSpacing.GetValue();

	m_vBackColorRed.GetValue();
	m_vBackColorGreen.GetValue();
	m_vBackColorBlue.GetValue();
	m_vSubItemLeftSpacing.GetValue();

	for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
	{
		CSTXListControlExItem *pItem = m_arrItems[i].get();
		pItem->UpdateVariableTime();
	}

	Invalidate();
	CWnd::OnTimer(nIDEvent);
}

void CSTXListControlEx::DrawControl(CDC *pDC)
{
	CRect rcThis;
	GetClientRect(rcThis);
	Gdiplus::Rect rectThis(rcThis.left, rcThis.top, rcThis.Width(), rcThis.Height());

	Gdiplus::Graphics g(pDC->GetSafeHdc());
	Gdiplus::Bitmap bmpMem(rcThis.Width(), rcThis.Height());
	Gdiplus::Graphics *pMemGraphics = Gdiplus::Graphics::FromImage(&bmpMem);

	DrawBackground(pMemGraphics);

	/*
	if(m_pImgBackground)
	{
		if(!m_pImgBackgroundCached)
		{
			Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(rcThis.Width(), rcThis.Height()); 
			Gdiplus::Graphics graphics(pBitmap);
			graphics.DrawImage(m_pImgBackground.get(), 0, 0, rcThis.Width(), rcThis.Height());

			Gdiplus::Bitmap *pBitmapSrc = pBitmap;
			tr1::shared_ptr<Gdiplus::CachedBitmap> imgCached(new Gdiplus::CachedBitmap(pBitmapSrc, pMemGraphics));
			m_pImgBackgroundCached = imgCached;

			delete pBitmap;
		}
		if(m_pImgBackgroundCached)
			pMemGraphics->DrawCachedBitmap(m_pImgBackgroundCached.get(), 0, 0);
		else
			pMemGraphics->DrawImage(m_pImgBackground.get(), rcThis.left, rcThis.top, rcThis.Width(), rcThis.Height());
	}
	else
	{
		Gdiplus::SolidBrush brushBk(Gdiplus::Color(
		 		(FLOAT)m_vBackColorRed.GetValue() * 255,
		 		(FLOAT)m_vBackColorGreen.GetValue() * 255,
		 		(FLOAT)m_vBackColorBlue.GetValue() * 255));
		pMemGraphics->FillRectangle(&brushBk, rcThis.left, rcThis.top, rcThis.Width(), rcThis.Height());
	}
	*/


	// Draw Top Reserved Space
	if(GetTopReservedHeight() > 0)
	{
		DrawTopReservedSpace(pMemGraphics);
	}

	// Draw Deleted Items
	size_t nDrawDelete = m_queueDeleted.size();
	for(size_t i=0;i<nDrawDelete;i++)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_queueDeleted.front();
		m_queueDeleted.pop();

		int top = static_cast<int>(floor(pItem->m_iOffset.GetValue() + 0.5));
		top -= GetScrollPos(SB_VERT);

		//Skip the items which is out of the visible area.
		if(top > rcThis.bottom)
			break;

		int left = (int)m_vLeftSpacing.GetValue();
		int nItemHeight = pItem->GetHeight();
		int fItemWidth = (int)(rcThis.Width() - m_vLeftSpacing.GetValue() - m_vRightSpacing.GetValue());

		pItem->DrawItemEx(pMemGraphics, left, top, nItemHeight, rcThis.Height());

		if(pItem->m_iItemOpacity.GetValue() > 0.02f)
		{
			m_queueDeleted.push(pItem);
		}
	}

	// Draw All Valid Items.
	for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
	{
		CSTXListControlExItem *pItem = m_arrItems[i].get();
		
		int top = static_cast<int>(floor(pItem->m_iOffset.GetValue() + 0.5));
		top -= GetScrollPos(SB_VERT);

		//Skip the items which is out of the visible area.
		if(top > rcThis.bottom)
			break;

		CSTXListControlExItem *pItemNext = NULL;
		if(i < m_arrItems.GetCount() - 1)
		{
			pItemNext = m_arrItems[i + 1].get();
			if((FLOAT)pItem->m_iOffset.GetValue() < 0)
				continue;
		}		
		
		int left = (int)m_vLeftSpacing.GetValue();
		int nItemHeight = pItem->GetHeight();
		int fItemWidth = (int)(rcThis.Width() - m_vLeftSpacing.GetValue() - m_vRightSpacing.GetValue());

		if(pItemNext)
		{
			//fItemHeight = (FLOAT)max(fItemHeight, pItemNext->m_iOffset.GetValue() - pItem->m_iOffset.GetValue());
			nItemHeight = (int)(pItemNext->m_iOffset.GetValue() - pItem->m_iOffset.GetValue());
		}

		//dcMem.SetViewportOrg(left, top);
		pItem->DrawItemEx(pMemGraphics, left, top, nItemHeight, rcThis.Height());
		//dcMem.SetViewportOrg(0, 0);
	}

	if(m_arrItems.GetCount() == 0)
	{
		//Draw Empty text
		Gdiplus::FontFamily  fontFamily(L"Times New Roman");
		Gdiplus::Font drawFont(&fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 0, 0, 0));
		Gdiplus::PointF ptText(4, 20);
		pMemGraphics->DrawString(m_strTextWhenEmpty, -1, &drawFont, ptText, &textBrush);
	}

	delete pMemGraphics;

	Gdiplus::TextureBrush brushContent(&bmpMem);
	//Gdiplus::CachedBitmap bmpMemCache(&bmpMem, &g);
	//g.DrawCachedBitmap(&bmpMemCache, 0, 0);
	//g.DrawImage(&bmpMem, 0, 0);
	g.FillRectangle(&brushContent, rectThis);
	g.ReleaseHDC(pDC->GetSafeHdc());
}

INT_PTR CSTXListControlEx::InsertNewSubItemCopy(INT_PTR iItemIndex, INT_PTR iSubItemIndex, std::tr1::shared_ptr<CSTXListControlExItem> pSubItem)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	CSTXListControlExItem *pItem = m_arrItems[iItemIndex].get();

	INT_PTR iInserted = pItem->InsertNewSubItemCopy(iSubItemIndex, pSubItem);
	if(iInserted == -1)
		return iInserted;

	FLOAT fExtra = 0.0f;
	if(pItem->m_arrChildItems.GetCount() == 1)	//First Child, need extra spacing
		fExtra = (FLOAT)pItem->m_vChildItemSpacing.GetFinalValue();

	if(pItem->m_bExpand)
	{
		//Adjust all items after iIndex
		for(INT_PTR i=iItemIndex + 1;i<m_arrItems.GetCount();i++)
		{
			CSTXListControlExItem *pItem = m_arrItems[i].get();
			pItem->m_iOffset = pItem->m_iOffset.GetFinalValue() + DEFAULT_ITEM_HEIGHT + pItem->m_vChildItemSpacing.GetValue() + fExtra;
		}

		ResetScrollBars();
	}

	Invalidate();

	if(m_iFocusItemIndex == iItemIndex && iSubItemIndex <= m_iFocusSubItemIndex)
		m_iFocusSubItemIndex++;

	return iInserted;
}

INT_PTR CSTXListControlEx::InsertNewSubItem(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszCaption)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	INT_PTR iInserted = pItem->InsertNewSubItem(iSubItemIndex, lpszCaption);
	if(iInserted == -1)
		return iInserted;

	std::tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iInserted];
	pSubItem->m_iLeftPadding = m_vSubItemLeftSpacing.GetFinalValue();

	FLOAT fExtra = 0.0f;
	if(pItem->m_arrChildItems.GetCount() == 1)	//First Child, need extra spacing
		fExtra = (FLOAT)pItem->m_vChildItemSpacing.GetFinalValue();

	if(pItem->m_bExpand)
	{
		//Adjust all items after iIndex
		for(INT_PTR i=iItemIndex + 1;i<m_arrItems.GetCount();i++)
		{
			std::tr1::shared_ptr<CSTXListControlExItem> pSubItemAdjust = m_arrItems[i];
			pSubItemAdjust->m_iOffset = pSubItemAdjust->m_iOffset.GetFinalValue() + DEFAULT_ITEM_HEIGHT + pSubItemAdjust->m_vChildItemSpacing.GetValue() + fExtra;
		}
	
		ResetScrollBars();
	}

	Invalidate();

	if(m_iFocusItemIndex == iItemIndex && iSubItemIndex <= m_iFocusSubItemIndex)
		m_iFocusSubItemIndex++;

	return iInserted;
}

INT_PTR CSTXListControlEx::InsertNewItem( INT_PTR iItemIndex, LPCTSTR lpszCaption /*= NULL*/ )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount())
		return -1;

	//Calculate the Initialize and Target offset
	CRect rcThis;
	GetClientRect(rcThis);

// 	DOUBLE fImageHeight = rcThis.Height() - m_vTopSpacing.GetValue() - m_vBottomSpacing.GetValue();
// 	DOUBLE fItemWidth = fImageHeight + m_vImageAndTextSpacing.GetValue() + m_vTextAreaWidth.GetValue();

	DOUBLE fOffsetTarget = 0.0;
	DOUBLE fOffsetInit = 0.0;

	if(iItemIndex == m_arrItems.GetCount())		//Insert at the end (append)
	{
		//Calculate the height of existing items, including item spaces
		int nTotalHeight = GetTopReservedHeight();
		for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			nTotalHeight += (pItem->GetHeight() + (int)m_vItemSpacing.GetFinalValue());
		}

		fOffsetTarget = m_vTopSpacing.GetValue() + nTotalHeight;
		fOffsetInit = m_vTopSpacing.GetValue() + nTotalHeight + DEFAULT_ITEM_HEIGHT;
	}
	else	//Insert in the imddle or beginning
	{
		//Calculate the height of existing items before insertion point, including item spaces
		int nTotalHeight = GetTopReservedHeight();
		for(INT_PTR i= 0;i<iItemIndex;i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			nTotalHeight += (pItem->GetHeight() + (int)m_vItemSpacing.GetFinalValue());
		}

		fOffsetTarget = m_vTopSpacing.GetValue() + nTotalHeight;
		fOffsetInit = fOffsetTarget - 4;

		//Adjust all the offsets after this item
		for(INT_PTR i=iItemIndex;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			pItem->m_iOffset = pItem->m_iOffset.GetFinalValue() + DEFAULT_HEADER_HEIGHT + m_vItemSpacing.GetFinalValue();
		}
	}

	std::tr1::shared_ptr<CSTXListControlExItem> pNewItem (new CSTXListControlExItem(this, NULL, fOffsetInit));

	if(lpszCaption)
		pNewItem->m_strCaption = lpszCaption;
	//pNewItem->m_iOffset.Reinitialize(fOffsetInit);
	pNewItem->m_iOffset = fOffsetTarget;
	pNewItem->m_iItemOpacity = 1.0f;

	m_arrItems.InsertAt(iItemIndex, pNewItem);

	if(iItemIndex <= m_iFocusItemIndex)
		m_iFocusItemIndex++;

	ResetScrollBars();


	Invalidate();


	return iItemIndex;
}

void CSTXListControlEx::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	DrawControl(&dc);
}

int CSTXListControlEx::GetItemWidth()
{
	CRect rcThis;
	GetClientRect(rcThis);
	return (int)(rcThis.Width() - m_vLeftSpacing.GetFinalValue() - m_vRightSpacing.GetFinalValue());
}

INT_PTR CSTXListControlEx::DeleteItem( INT_PTR iItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	//Calculate the Initialize and Target offset
	CRect rcThis;
	GetClientRect(rcThis);

	DOUBLE fImageHeight = rcThis.Height() - m_vTopSpacing.GetValue() - m_vBottomSpacing.GetValue();

	tr1::shared_ptr<CSTXListControlExItem> pItemToDelete = m_arrItems[iItemIndex];

	int iCurrentPos = (int)pItemToDelete->m_iOffset.GetFinalValue();

	//Adjust all the offsets after this item
	for(INT_PTR i=iItemIndex + 1;i<m_arrItems.GetCount();i++)
	{
		tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
		pItem->m_iOffset = iCurrentPos;
		iCurrentPos += (int)(pItem->GetHeight() + m_vItemSpacing.GetFinalValue());
	}

	
	m_queueDeleted.push(pItemToDelete);
	pItemToDelete->m_iItemOpacity = 0.0f;
	pItemToDelete->m_iContentOpacity = 0.0f;

	m_arrItems.RemoveAt(iItemIndex);

	ResetScrollBars();

	Invalidate();

	if(iItemIndex == m_iFocusItemIndex)
		SetFocusedItem(-1, -1);
	else if(iItemIndex < m_iFocusItemIndex)
		m_iFocusItemIndex--;

	return iItemIndex;
}

INT_PTR CSTXListControlEx::MoveSubItemToEnd(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	tr1::shared_ptr<CSTXListControlExItem> pSubItem = DeleteSubItemGetCopy(iItemIndex, iSubItemIndex);
	return InsertNewSubItemCopy(iItemIndex, pItem->m_arrChildItems.GetCount(), pSubItem);
}

INT_PTR CSTXListControlEx::MoveSubItemToBegin(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	tr1::shared_ptr<CSTXListControlExItem> pSubItem = DeleteSubItemGetCopy(iItemIndex, iSubItemIndex);
	return InsertNewSubItemCopy(iItemIndex, 0, pSubItem);
}

std::tr1::shared_ptr<CSTXListControlExItem> CSTXListControlEx::DeleteSubItemGetCopy(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return std::tr1::shared_ptr<CSTXListControlExItem>();

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	int nCount = pItem->m_arrChildItems.GetCount();
	if(iSubItemIndex < 0 || iSubItemIndex > nCount - 1)
		return std::tr1::shared_ptr<CSTXListControlExItem>();

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	INT_PTR iDelete = pItem->DeleteSubItem(iSubItemIndex, FALSE);

	int iExtra = 0;
	if(iDelete >= 0 && pItem->m_arrChildItems.GetCount() == 0)
		iExtra = (int)pItem->m_vChildItemSpacing.GetFinalValue();

	if(iDelete != -1 && pItem->m_bExpand)
	{
		//Adjust all items after iIndex
		for(INT_PTR i=iItemIndex + 1;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			pItem->m_iOffset = pItem->m_iOffset.GetFinalValue() - (DEFAULT_ITEM_HEIGHT + pItem->m_vChildItemSpacing.GetValue()) - iExtra;
		}
		ResetScrollBars();
	}

	if(m_iFocusItemIndex == iItemIndex)
	{
		if(m_iFocusSubItemIndex == iSubItemIndex)
		{
			SetFocusedItem(-1, -1);
		}
		else if(m_iFocusSubItemIndex > iSubItemIndex)
		{
			m_iFocusSubItemIndex--;
		}
	}

	Invalidate();

	return pSubItem;
}

INT_PTR CSTXListControlEx::DeleteSubItem( INT_PTR iItemIndex, INT_PTR iSubItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	INT_PTR iDelete = pItem->DeleteSubItem(iSubItemIndex, TRUE);

	int iExtra = 0;
	if(iDelete >= 0 && pItem->m_arrChildItems.GetCount() == 0)
		iExtra = (int)pItem->m_vChildItemSpacing.GetFinalValue();

	if(iDelete != -1 && pItem->m_bExpand)
	{
		//Adjust all items after iIndex
		for(INT_PTR i=iItemIndex + 1;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			pItem->m_iOffset = pItem->m_iOffset.GetFinalValue() - (DEFAULT_ITEM_HEIGHT + pItem->m_vChildItemSpacing.GetValue()) - iExtra;
		}
		ResetScrollBars();
	}

	if(m_iFocusItemIndex == iItemIndex)
	{
		if(m_iFocusSubItemIndex == iSubItemIndex)
		{
			SetFocusedItem(-1, -1);
		}
		else if(m_iFocusSubItemIndex > iSubItemIndex)
		{
			m_iFocusSubItemIndex--;
		}
	}

	if(IsWindow(m_hWnd))
		Invalidate();

	return iDelete;
}

INT_PTR CSTXListControlEx::GetItemCount() const
{
	return m_arrItems.GetCount();
}

DWORD_PTR CSTXListControlEx::GetItemData( INT_PTR iItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return 0;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	return pItem->m_dwItemData;
}

INT_PTR CSTXListControlEx::SetItemData( INT_PTR iItemIndex, DWORD_PTR dwItemData )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	pItem->m_dwItemData = dwItemData;

	return iItemIndex;
}

int CSTXListControlEx::GetTotalHeight()
{
	//TODO: Add Cache to improve performance.

	return CalculateTotalHeight();
}

int CSTXListControlEx::CalculateTotalHeight()
{
	int nTotalHeight = (int)m_vTopSpacing.GetFinalValue();
	for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
	{
		nTotalHeight += m_arrItems[i]->GetHeight();
		nTotalHeight += (int)m_vItemSpacing.GetFinalValue();
	}
	nTotalHeight += (int)m_vBottomSpacing.GetFinalValue();
	return nTotalHeight;
}

void CSTXListControlEx::ResetScrollBars()
{
	if(!IsWindow(m_hWnd))
		return;

	CRect rcClient;
	GetClientRect(rcClient);

	int iTotalHeightAvailable = rcClient.Height() - GetTopReservedHeight();

	int iCurPos = 0;
	BOOL bVScrollExist;
	if((GetStyle() & WS_VSCROLL) == WS_VSCROLL)
	{
		iCurPos = GetScrollPos(SB_VERT);
		bVScrollExist = TRUE;
	}
	else
		bVScrollExist = FALSE;

	int iOldPos = GetScrollPos(SB_VERT);

	int nTotalHeight = GetTotalHeight();
	if(nTotalHeight > iTotalHeightAvailable)	//Need H-ScrollBar
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
		si.nPage = iTotalHeightAvailable;
		si.nMin = 0;
		si.nMax = nTotalHeight;
		si.nPos = min(iCurPos,si.nMax);

		SetScrollPos(SB_VERT,si.nPos);
		SetScrollInfo(SB_VERT,&si);
		EnableScrollBarCtrl(SB_VERT, TRUE);
		ModifyStyle(0,WS_VSCROLL);

	}
	else
	{
		int iCurPos = GetScrollPos(SB_VERT);
		ScrollWindow(iCurPos,0);
		SetScrollPos(SB_VERT,0);
		EnableScrollBarCtrl(SB_VERT, FALSE);
		ModifyStyle(WS_VSCROLL,0);
	}

	Invalidate(FALSE);
}


void CSTXListControlEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// Get the minimum and maximum scroll-bar positions.
	int minpos;
	int maxpos;
	GetScrollRange(SB_VERT, &minpos, &maxpos); 
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(SB_VERT);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos-=5;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos+=5;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
		{
			// Get the page size. 
			SCROLLINFO   info;
			GetScrollInfo(SB_VERT, &info, SIF_ALL);

			if (curpos > minpos)
				curpos = max(minpos, curpos - (int) info.nPage);
		}
		break;

	case SB_PAGERIGHT:      // Scroll one page right.
		{
			// Get the page size. 
			SCROLLINFO   info;
			GetScrollInfo(SB_VERT, &info, SIF_ALL);

			if (curpos < maxpos)
				curpos = min(maxpos, curpos + (int) info.nPage);
		}
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	SetScrollPos(SB_VERT, curpos);

	Invalidate();

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CSTXListControlEx::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_pImgBackgroundCached = std::tr1::shared_ptr<Gdiplus::CachedBitmap>();
	ResetScrollBars();
	Invalidate();
}

INT_PTR CSTXListControlEx::ItemHitTest(CPoint point, BOOL *pbInHeader)
{
	CRect rcThis;
	GetClientRect(rcThis);

	FLOAT fItemWidth = (FLOAT)(rcThis.Width() - m_vLeftSpacing.GetValue() - m_vRightSpacing.GetValue());

	for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
	{
		tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
		CRect rcItem((int)m_vLeftSpacing.GetValue(), (int)pItem->m_iOffset.GetValue(), 0, 0);
		rcItem.right = rcItem.left + (int)fItemWidth;
		rcItem.bottom = rcItem.top + pItem->GetHeight();

		CRect rcItemHeaderOnly = rcItem;
		rcItemHeaderOnly.bottom = rcItemHeaderOnly.top + pItem->GetHeaderHeight();


		rcItem.OffsetRect(0, -GetScrollPos(SB_VERT));

		if(PtInRect(rcItem, point))
		{
			rcItemHeaderOnly.OffsetRect(0, -GetScrollPos(SB_VERT));
			if(pbInHeader)
				*pbInHeader = PtInRect(rcItemHeaderOnly, point);

			return i;
		}
	}

	return -1;
}

INT_PTR CSTXListControlEx::SubItemHitTest(INT_PTR iItemIndex, CPoint point)
{
	//Convert to Item local axis
	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	point.x -= (int)m_vLeftSpacing.GetValue();
	point.y += GetScrollPos(SB_VERT);
	point.y -= (int)pItem->m_iOffset.GetValue();

	//Now local point

	for(INT_PTR i=0;i<pItem->m_arrChildItems.GetCount();i++)
	{
		CRect rcSubItem;
		pItem->GetSubItemRect(i, rcSubItem);

		if(PtInRect(rcSubItem, point))
			return i;
	}

	return -1;
}

void CSTXListControlEx::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.hwndTrack = GetSafeHwnd();
	tme.dwFlags = TME_LEAVE;
	TrackMouseEvent(&tme);


	BOOL bInHeader = FALSE;
	INT_PTR iHit = ItemHitTest(point, &bInHeader);
	if(iHit != -1)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pHitNow = m_arrItems[iHit];
		INT_PTR iHitSubItem = SubItemHitTest(iHit, point);
		if(iHitSubItem != -1)
			pHitNow = m_arrItems[iHit]->m_arrChildItems[iHitSubItem];
		else if(!bInHeader)
			pHitNow = std::tr1::shared_ptr<CSTXListControlExItem>();

		if(m_ptLButtonDown.x >= 0)
		{
			if(abs(point.x - m_ptLButtonDown.x) > 3 || abs(point.y - m_ptLButtonDown.y) > 3)
			{
				if(!m_bDragging)
				{
					m_bDragging = TRUE;
					OnBeginDrag(pHitNow);
				}
			}
		}

		if(m_pLastHoverItem != pHitNow)
		{
			if(m_pLastHoverItem != NULL)
				m_pLastHoverItem->m_iHoverMarkOpacity = 0.0;

			m_pLastHoverItem = pHitNow;

			if(m_pLastHoverItem)
			{
				m_pLastHoverItem->m_iHoverMarkOpacity = 0.8;
			}


			Invalidate();
		}
	}
	else
	{
		if(m_pLastHoverItem != NULL)
		{
			m_pLastHoverItem->m_iHoverMarkOpacity = 0.0;
			m_pLastHoverItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL


			Invalidate();
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CSTXListControlEx::OnMouseLeave()
{
//	m_ptLButtonDown.x = -1;
//	m_bDragging = FALSE;

 	if(m_pLastLMouseDownButton != NULL)
 		m_pLastLMouseDownButton = std::tr1::shared_ptr<CSTXListControlExButton>();	//NULL

 	if(m_pLastLMouseDownItem != NULL)
 		m_pLastLMouseDownItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL
 	if(m_pLastRMouseDownItem != NULL)
 		m_pLastRMouseDownItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL

	if(m_pLastHoverItem != NULL)
	{
		m_pLastHoverItem->m_iHoverMarkOpacity = 0.0;
		m_pLastHoverItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL

		Invalidate();
	}

	CWnd::OnMouseLeave();
}


BOOL CSTXListControlEx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	BOOL bInHeader = FALSE;
	INT_PTR iHit = ItemHitTest(point, &bInHeader);
	if(iHit != -1)
	{
		SetCursor(::LoadCursor(NULL, IDC_HAND));
		//::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(32649)));
	}
	else
	{
		SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
	}

	return FALSE;
	//return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CSTXListControlEx::SetFocusedItem(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	INT_PTR iNewFocusItemIndex = -1;
	INT_PTR iNewFocusSubItemIndex = -1;

	tr1::shared_ptr<CSTXListControlExItem> pNewFocusItem;
	if(iItemIndex < 0)
	{
		// NULL
	}
	else if(iItemIndex > m_arrItems.GetCount() - 1)
	{
		// NULL
	}
	else
	{
		if(iSubItemIndex < 0)
		{
			pNewFocusItem = m_arrItems[iItemIndex];
			iNewFocusItemIndex = iItemIndex;
		}
		else
		{
			iNewFocusItemIndex = iItemIndex;
			iSubItemIndex = min(iSubItemIndex, m_arrItems[iItemIndex]->m_arrChildItems.GetCount() - 1);
			iNewFocusSubItemIndex = iSubItemIndex;
			pNewFocusItem = m_arrItems[iItemIndex]->m_arrChildItems[iSubItemIndex];
		}
	}

	if(m_pFocusedItem != pNewFocusItem)
	{
		m_pFocusedItem = pNewFocusItem;
		m_iFocusItemIndex = iNewFocusItemIndex;
		m_iFocusSubItemIndex = iNewFocusSubItemIndex;

		//Scroll Focus item to view
		InternalEnsureVisible(pNewFocusItem);

		Invalidate();
	}
}

void CSTXListControlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(m_arrItems.GetCount() == 0)
	{
		NMHDR hdr;
		hdr.hwndFrom = GetSafeHwnd();
		hdr.idFrom = GetDlgCtrlID();
		hdr.code = STXALN_EMPTYCLICK;
		LRESULT result = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&hdr);

		CWnd::OnLButtonDown(nFlags, point);
		return;
	}

	m_bDragging = FALSE;
	SetFocus();

	BOOL bInHeader = FALSE;
	INT_PTR iHit = ItemHitTest(point, &bInHeader);
	if(iHit != -1)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pHitNow = m_arrItems[iHit];
		INT_PTR iHitSubItem = SubItemHitTest(iHit, point);
		if(iHitSubItem != -1)
			pHitNow = m_arrItems[iHit]->m_arrChildItems[iHitSubItem];

		if(m_pLastLMouseDownItem != pHitNow)
		{
			m_pLastLMouseDownItem = pHitNow;
		}

		SetFocusedItem(iHit, iHitSubItem);
		
		SetCapture();
		m_ptLButtonDown = point;

		if(pHitNow)
		{
			CPoint ptInItem = point;
			ptInItem.y += GetScrollPos(SB_VERT);
			if(pHitNow->m_pParentItem == NULL)
				ptInItem.Offset(0, -(int)pHitNow->m_iOffset.GetValue());
			else
			{
				ptInItem.Offset(0, -(int)pHitNow->m_pParentItem->m_iOffset.GetValue());
				ptInItem.Offset(0, -(int)pHitNow->m_iOffset.GetValue());
			}

			INT_PTR iBtnHit = pHitNow->ButtonHitTest(ptInItem);
			if(iBtnHit != -1)
			{
				std::tr1::shared_ptr<CSTXListControlExButton> pBtnHit = pHitNow->GetButton(iBtnHit);
				m_pLastLMouseDownButton = pBtnHit;
			}
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}


void CSTXListControlEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_ptLButtonDown.x = -1;
	m_bDragging = FALSE;

	BOOL bInHeader = FALSE;
	INT_PTR iHit = ItemHitTest(point, &bInHeader);
	if(iHit != -1)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pHitNow = m_arrItems[iHit];
		INT_PTR iHitSubItem = SubItemHitTest(iHit, point);
		if(iHitSubItem != -1)
			pHitNow = m_arrItems[iHit]->m_arrChildItems[iHitSubItem];

		BOOL bProcessItemClickEvent = TRUE;
		if(pHitNow)
		{
			CPoint ptInItem = point;
			ptInItem.y += GetScrollPos(SB_VERT);
			if(pHitNow->m_pParentItem == NULL)
				ptInItem.Offset(0, -(int)pHitNow->m_iOffset.GetValue());
			else
			{
				ptInItem.Offset(0, -(int)pHitNow->m_pParentItem->m_iOffset.GetValue());
				ptInItem.Offset(0, -(int)pHitNow->m_iOffset.GetValue());
			}

			INT_PTR iBtnHit = pHitNow->ButtonHitTest(ptInItem);
			if(iBtnHit != -1)
			{
				std::tr1::shared_ptr<CSTXListControlExButton> pBtnHit = pHitNow->GetButton(iBtnHit);
				if(pBtnHit == m_pLastLMouseDownButton)
				{
					//Trigger Mouse Click Event
					NMSTXALBUTTONCLICK nm;
					nm.hdr.hwndFrom = GetSafeHwnd();
					nm.hdr.idFrom = GetDlgCtrlID();
					nm.hdr.code = STXALN_BUTTONCLICK;
					nm.iIndex = iHit;
					nm.iSubIndex = iHitSubItem;
					nm.nMouseButton = 0;	//Left Button
					nm.dwItemData = pHitNow->m_dwItemData;
					nm.iButtonIndex = iBtnHit;
					LRESULT result = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm);
					bProcessItemClickEvent = FALSE;
				}
			}
		}
		m_pLastLMouseDownButton = std::tr1::shared_ptr<CSTXListControlExButton>();	//NULL

		if(m_pLastLMouseDownItem == pHitNow && bProcessItemClickEvent)
		{
			//Trigger Mouse Click Event
			NMSTXALITEMCLICK nm;
			nm.hdr.hwndFrom = GetSafeHwnd();
			nm.hdr.idFrom = GetDlgCtrlID();
			nm.hdr.code = STXALN_ITEMCLICK;
			nm.iIndex = iHit;
			nm.bInHeader = bInHeader;
			nm.iSubIndex = iHitSubItem;
			nm.nMouseButton = 0;	//Left Button
			nm.dwItemData = m_pLastLMouseDownItem->m_dwItemData;

			LRESULT result = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm);

			//CSTXAnimationManager::ScheduleAnimation();
			//Invalidate();
		}
		m_pLastLMouseDownItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL
	}
	else
	{
		if(m_pLastLMouseDownItem == NULL)
		{
			NMHDR hdr;
			hdr.hwndFrom = GetSafeHwnd();
			hdr.idFrom = GetDlgCtrlID();
			hdr.code = STXALN_CLICK;
			LRESULT result = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&hdr);
		}
	}

	CWnd::OnLButtonUp(nFlags, point);
}

CString CSTXListControlEx::GetSubItemCaption( INT_PTR iItemIndex, INT_PTR iSubItemIndex, BOOL bSubCaption )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return CString(_T(""));

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return CString(_T(""));

	if(bSubCaption)
		return pItem->m_arrChildItems[iSubItemIndex]->m_strSubCaption;

	return pItem->m_arrChildItems[iSubItemIndex]->m_strCaption;
}

CString CSTXListControlEx::GetItemCaption( INT_PTR iItemIndex, BOOL bSubCaption )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return CString(_T(""));

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(bSubCaption)
		return pItem->m_strSubCaption;

	return pItem->m_strCaption;
}


void CSTXListControlEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	BOOL bInHeader = FALSE;
	INT_PTR iHit = ItemHitTest(point, &bInHeader);
	if(iHit != -1)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pHitNow = m_arrItems[iHit];
		INT_PTR iHitSubItem = SubItemHitTest(iHit, point);
		if(iHitSubItem != -1)
			pHitNow = m_arrItems[iHit]->m_arrChildItems[iHitSubItem];

		if(m_pLastRMouseDownItem != pHitNow)
		{
			m_pLastRMouseDownItem = pHitNow;
		}

		SetFocusedItem(iHit, iHitSubItem);
	}

	CWnd::OnRButtonDown(nFlags, point);
}


void CSTXListControlEx::OnRButtonUp(UINT nFlags, CPoint point)
{
	BOOL bInHeader = FALSE;
	INT_PTR iHit = ItemHitTest(point, &bInHeader);
	if(iHit != -1)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pHitNow = m_arrItems[iHit];
		INT_PTR iHitSubItem = SubItemHitTest(iHit, point);
		if(iHitSubItem != -1)
			pHitNow = m_arrItems[iHit]->m_arrChildItems[iHitSubItem];

		if(m_pLastRMouseDownItem == pHitNow)
		{
			//Trigger Mouse Click Event
			NMSTXALITEMCLICK nm;
			nm.hdr.hwndFrom = GetSafeHwnd();
			nm.hdr.idFrom = GetDlgCtrlID();
			nm.hdr.code = STXALN_ITEMCLICK;
			nm.iIndex = iHit;
			nm.bInHeader = bInHeader;
			nm.iSubIndex = iHitSubItem;
			nm.nMouseButton = 1;	//Right Button
			nm.dwItemData = m_pLastRMouseDownItem->m_dwItemData;

			LRESULT result = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm);


			//CSTXAnimationManager::ScheduleAnimation();
			//Invalidate();
		}
		m_pLastRMouseDownItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL
	}
	else
	{
		if(m_pLastRMouseDownItem == NULL)
		{
			NMHDR hdr;
			hdr.hwndFrom = GetSafeHwnd();
			hdr.idFrom = GetDlgCtrlID();
			hdr.code = STXALN_RCLICK;
			LRESULT result = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&hdr);
		}
	}

	CWnd::OnRButtonUp(nFlags, point);
}

void CSTXListControlEx::ExpandItem(INT_PTR iItemIndex, BOOL bExpand)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	BOOL bOldExpand = pItem->m_bExpand;
	if(bOldExpand != bExpand)
	{
		if(bOldExpand)
		{
			int iHeightOld = pItem->GetHeight();
			int iHeightNew = pItem->GetHeaderHeight();

			//Adjust all the offsets after this item
			for(INT_PTR i=iItemIndex + 1;i<m_arrItems.GetCount();i++)
			{
				CSTXListControlExItem *pItem = m_arrItems[i].get();
				pItem->m_iOffset = pItem->m_iOffset.GetFinalValue() + (iHeightNew - iHeightOld);
			}
			pItem->m_bExpand = FALSE;
			pItem->m_iContentOpacity = 0.0f;
		}
		else
		{
			int iHeightOld = pItem->GetHeaderHeight();
			pItem->m_bExpand = TRUE;
			int iHeightNew = pItem->GetHeight();

			//Adjust all the offsets after this item
			for(INT_PTR i=iItemIndex + 1;i<m_arrItems.GetCount();i++)
			{
				CSTXListControlExItem *pItem = m_arrItems[i].get();
				pItem->m_iOffset = pItem->m_iOffset.GetFinalValue() + (iHeightNew - iHeightOld);
			}
			pItem->m_iContentOpacity = 1.0f;
		}
		ResetScrollBars();

		Invalidate();
	}
}

BOOL CSTXListControlEx::IsItemExpanded( INT_PTR iItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return FALSE;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	return pItem->m_bExpand;
}

HRESULT CSTXListControlEx::SetItemImage(INT_PTR iItemIndex, LPCTSTR lpszFile, BOOL bExpanded /* = TRUE */ )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return E_FAIL;

	CSTXListControlExItem *pItem = m_arrItems[iItemIndex].get();

	HRESULT hr = S_OK;
	
	std::tr1::shared_ptr<Gdiplus::Image> *pImageToLoad = NULL;
	if(bExpanded)
	{
		pImageToLoad = &pItem->m_pImgImage;
	}
	else
	{
		pImageToLoad = &pItem->m_pImgImageCollapse;
	}

	*pImageToLoad = std::tr1::shared_ptr<Gdiplus::Image>();	//NULL
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(lpszFile));
	*pImageToLoad = img;


	if(SUCCEEDED(hr))
		Invalidate();

	return hr;
}

HRESULT CSTXListControlEx::SetItemImage( INT_PTR iItemIndex, HBITMAP hBitmap, BOOL bExpanded /* = TRUE */ )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return E_FAIL;

	CSTXListControlExItem *pItem = m_arrItems[iItemIndex].get();


	HRESULT hr = S_OK;

	std::tr1::shared_ptr<Gdiplus::Image> *pImageToLoad = NULL;
	if(bExpanded)
	{
		pImageToLoad = &pItem->m_pImgImage;
	}
	else
	{
		pImageToLoad = &pItem->m_pImgImageCollapse;
	}
	*pImageToLoad = std::tr1::shared_ptr<Gdiplus::Image>();	//NULL
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	*pImageToLoad = img;

	if(SUCCEEDED(hr))
		Invalidate();

	return hr;
}

HRESULT CSTXListControlEx::SetItemImage( INT_PTR iItemIndex, IStream *pStream, BOOL bExpanded /* = TRUE */ )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return E_FAIL;

	CSTXListControlExItem *pItem = m_arrItems[iItemIndex].get();

	HRESULT hr = S_OK;

	std::tr1::shared_ptr<Gdiplus::Image> *pImageToLoad = NULL;
	if(bExpanded)
	{
		pImageToLoad = &pItem->m_pImgImage;
	}
	else
	{
		pImageToLoad = &pItem->m_pImgImageCollapse;
	}
	int iHeaderHeight = pItem->GetHeaderHeight();
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(pStream));
	UINT o_height = img->GetHeight();
	UINT o_width = img->GetWidth();

	if(o_height > iHeaderHeight || o_width > iHeaderHeight)
	{
		INT n_width = iHeaderHeight;
		INT n_height = iHeaderHeight;
		double ratio = ((double)o_width) / ((double)o_height);
		if (o_width > o_height) 
		{
			// Resize down by width
			n_height = static_cast<UINT>(((double)n_width) / ratio);
		} 
		else
		{
			n_width = static_cast<UINT>(n_height * ratio);
		}

		tr1::shared_ptr<Gdiplus::Image> imgThumbnail(img->GetThumbnailImage(n_width, n_height));
		*pImageToLoad = imgThumbnail;
	}
	else
	{
		*pImageToLoad = img;
	}

	if(SUCCEEDED(hr))
		Invalidate();

	return hr;
}

HBITMAP CSTXListControlEx::CloneBitmap( HBITMAP hSourceBitmap )
{
    // Pointer to access the pixels of bitmap
    HDC hdcSrc = CreateCompatibleDC( NULL );
    HDC hdcDst = CreateCompatibleDC( NULL );
    HBITMAP hNewBitmap;
    BITMAP stBitmap = {0};
    GetObject( hSourceBitmap, sizeof(stBitmap), &stBitmap );
    hNewBitmap = CreateBitmap( stBitmap.bmWidth, stBitmap.bmHeight,
                                stBitmap.bmPlanes,stBitmap.bmBitsPixel,NULL);
    if( hNewBitmap )
    {
        SelectObject( hdcSrc, hSourceBitmap);
        SelectObject(hdcDst, hNewBitmap);
        // Blit existing bitmap to newly created bitmap
        BitBlt(hdcDst, 0, 0, stBitmap.bmWidth, stBitmap.bmHeight, hdcSrc, 0, 0, SRCCOPY);
    }

    // Cleanup the created Device contexts
    SelectObject(hdcSrc, 0 );
    SelectObject(hdcDst, 0 );
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    return hNewBitmap;
}

HRESULT CSTXListControlEx::SetSubItemImage( INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszFile )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return E_FAIL;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return E_FAIL;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	HRESULT hr = S_OK;

	pSubItem->m_pImgImage = std::tr1::shared_ptr<Gdiplus::Image>();	//NULL
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(lpszFile));
	pSubItem->m_pImgImage = img;

	if(SUCCEEDED(hr))
		Invalidate();

	return hr;
}

HRESULT CSTXListControlEx::SetSubItemImage( INT_PTR iItemIndex, INT_PTR iSubItemIndex, HBITMAP hBitmap )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return E_FAIL;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return E_FAIL;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	HRESULT hr = S_OK;

	pSubItem->m_pImgImage = std::tr1::shared_ptr<Gdiplus::Image>();	//NULL
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	pSubItem->m_pImgImage = img;

	if(SUCCEEDED(hr))
		Invalidate();

	return hr;
}

INT_PTR CSTXListControlEx::SetSubItemTag1(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszTag)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];
	pSubItem->m_strTag1 = lpszTag;
	return iSubItemIndex;
}

INT_PTR CSTXListControlEx::SetSubItemTag2(INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszTag)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];
	pSubItem->m_strTag2 = lpszTag;
	return iSubItemIndex;
}

CString CSTXListControlEx::GetSubItemTag1(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return _T("");

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return _T("");

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];
	return pSubItem->m_strTag1;
}

CString CSTXListControlEx::GetSubItemTag2(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return _T("");

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return _T("");

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];
	return pSubItem->m_strTag2;
}

INT_PTR CSTXListControlEx::FindSubItemWithTag1(INT_PTR *pItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive)
{
	if(bCaseSensitive)
	{
		for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			if(pItem->m_bExcludeFromFindSubItem)
				continue;
			for(INT_PTR k=0;k<pItem->m_arrChildItems.GetCount();k++)
			{
				tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[k];
				if(_tcscmp(pSubItem->m_strTag1, lpszTag) == 0)
				{
					if(pItemIndex)
						*pItemIndex = i;
					if(pSubItemIndex)
						*pSubItemIndex = k;
					return k;
				}
			}
		}
	}
	else
	{
		for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			if(pItem->m_bExcludeFromFindSubItem)
				continue;
			for(INT_PTR k=0;k<pItem->m_arrChildItems.GetCount();k++)
			{
				tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[k];
				if(_tcsicmp(pSubItem->m_strTag1, lpszTag) == 0)
				{
					if(pItemIndex)
						*pItemIndex = i;
					if(pSubItemIndex)
						*pSubItemIndex = k;
					return k;
				}
			}
		}
	}

	if(pItemIndex)
		*pItemIndex = -1;
	if(pSubItemIndex)
		*pSubItemIndex = -1;
	return -1;
}

INT_PTR CSTXListControlEx::FindSubItemWithTag1(INT_PTR iItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	for(INT_PTR k=0;k<pItem->m_arrChildItems.GetCount();k++)
	{
		tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[k];
		if(_tcscmp(pSubItem->m_strTag1, lpszTag) == 0)
		{
			if(pSubItemIndex)
				*pSubItemIndex = k;
			return k;
		}
	}

	if(pSubItemIndex)
		*pSubItemIndex = -1;

	return -1;
}

INT_PTR CSTXListControlEx::FindSubItemWithTag2(INT_PTR iItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	for(INT_PTR k=0;k<pItem->m_arrChildItems.GetCount();k++)
	{
		tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[k];
		if(_tcscmp(pSubItem->m_strTag2, lpszTag) == 0)
		{
			if(pSubItemIndex)
				*pSubItemIndex = k;
			return k;
		}
	}

	if(pSubItemIndex)
		*pSubItemIndex = -1;

	return -1;
}

INT_PTR CSTXListControlEx::FindSubItemWithTag2(INT_PTR *pItemIndex, INT_PTR *pSubItemIndex, LPCTSTR lpszTag, BOOL bCaseSensitive)
{
	if(bCaseSensitive)
	{
		for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			if(pItem->m_bExcludeFromFindSubItem)
				continue;
			for(INT_PTR k=0;k<pItem->m_arrChildItems.GetCount();k++)
			{
				tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[k];
				if(_tcscmp(pSubItem->m_strTag2, lpszTag) == 0)
				{
					if(pItemIndex)
						*pItemIndex = i;
					if(pSubItemIndex)
						*pSubItemIndex = k;
					return k;
				}
			}
		}
	}
	else
	{
		for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			if(pItem->m_bExcludeFromFindSubItem)
				continue;
			for(INT_PTR k=0;k<pItem->m_arrChildItems.GetCount();k++)
			{
				tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[k];
				if(_tcsicmp(pSubItem->m_strTag2, lpszTag) == 0)
				{
					if(pItemIndex)
						*pItemIndex = i;
					if(pSubItemIndex)
						*pSubItemIndex = k;
					return k;
				}
			}
		}
	}

	if(pItemIndex)
		*pItemIndex = -1;
	if(pSubItemIndex)
		*pSubItemIndex = -1;

	return -1;
}

/*
INT_PTR CSTXListControlEx::SetSubItemBusy(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	pSubItem->m_iMarkImage = 1;		//Busy
	return iSubItemIndex;
}

INT_PTR CSTXListControlEx::SetSubItemAway(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	pSubItem->m_iMarkImage = 2;		//Away
	return iSubItemIndex;
}
*/

INT_PTR CSTXListControlEx::ClearSubItemMark(INT_PTR iItemIndex, INT_PTR iSubItemIndex)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	pSubItem->m_iMarkImage = -1;
	return iSubItemIndex;
}

INT_PTR CSTXListControlEx::SetSubItemImageGray(INT_PTR iItemIndex, INT_PTR iSubItemIndex, BOOL bGray)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	pSubItem->UseGrayImage(bGray);
	return iSubItemIndex;
}

HRESULT CSTXListControlEx::SetSubItemImage( INT_PTR iItemIndex, INT_PTR iSubItemIndex, IStream *pStream)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return E_FAIL;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return E_FAIL;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	HRESULT hr = S_OK;

	pSubItem->m_pImgImage = GetResizedImage(pStream, DEFAULT_ITEM_HEIGHT);

	if(SUCCEEDED(hr))
		Invalidate();

	return hr;
}

INT_PTR CSTXListControlEx::SetSubItemCaption( INT_PTR iItemIndex, INT_PTR iSubItemIndex, LPCTSTR lpszCaption, BOOL bSubCaption)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];
	if(bSubCaption)
		pSubItem->m_strSubCaption = lpszCaption;
	else
		pSubItem->m_strCaption = lpszCaption;

	Invalidate();
	return iSubItemIndex;
}

void CSTXListControlEx::DeleteAllItems()
{
	m_iFocusItemIndex = -1;
	m_iFocusSubItemIndex = -1;
	m_pFocusedItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL

	m_pLastLMouseDownItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL
	m_pLastRMouseDownItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL
	m_pLastHoverItem = std::tr1::shared_ptr<CSTXListControlExItem>();	//NULL

	m_arrItems.RemoveAll();
	Invalidate();
}

HRESULT CSTXListControlEx::SetBackgroundImage( LPCTSTR lpszFile )
{
	HRESULT hr = S_OK;
	CRect rcThis;
	GetClientRect(rcThis);

	m_pImgBackground = std::tr1::shared_ptr<Gdiplus::Image>();	//NULL
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(lpszFile));
	m_pImgBackground = img;
	m_pImgBackgroundCached = std::tr1::shared_ptr<Gdiplus::CachedBitmap>();	//NULL

	if(SUCCEEDED(hr))
	{
		Invalidate();
	}

	return hr;
	
}
/*
HRESULT CSTXListControlEx::SetBusyImage(HBITMAP hBitmap)
{
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	m_pImgBusy = img;
	return S_OK;
}

HRESULT CSTXListControlEx::SetBusyImage(IStream *pStream)
{
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(pStream));
	m_pImgBusy = img;
	return S_OK;
}

HRESULT CSTXListControlEx::SetAwayImage(HBITMAP hBitmap)
{
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	m_pImgAway = img;
	return S_OK;
}

HRESULT CSTXListControlEx::SetAwayImage(IStream *pStream)
{
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(pStream));
	m_pImgAway = img;
	return S_OK;
}
*/

HRESULT CSTXListControlEx::SetBackgroundImage( HBITMAP hBitmap )
{
	HRESULT hr = S_OK;
	CRect rcThis;
	GetClientRect(rcThis);

	m_pImgBackground = std::tr1::shared_ptr<Gdiplus::Image>();	//NULL
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	m_pImgBackground = img;
	m_pImgBackgroundCached = std::tr1::shared_ptr<Gdiplus::CachedBitmap>();	//NULL

	if(SUCCEEDED(hr))
	{
		Invalidate();
	}

	return hr;
}

HRESULT CSTXListControlEx::SetBackgroundImage( IStream *pStream)
{
	HRESULT hr = S_OK;
	CRect rcThis;
	GetClientRect(rcThis);

	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(pStream));
	m_pImgBackground = img;
	m_pImgBackgroundCached = std::tr1::shared_ptr<Gdiplus::CachedBitmap>();	//NULL

	if(SUCCEEDED(hr))
	{
		Invalidate();
	}

	return hr;
}

INT_PTR CSTXListControlEx::SetSubItemData( INT_PTR iItemIndex, INT_PTR iSubItemIndex, DWORD_PTR dwItemData )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];
	pSubItem->m_dwItemData = dwItemData;

	return iSubItemIndex;
}

DWORD_PTR CSTXListControlEx::GetSubItemData( INT_PTR iItemIndex, INT_PTR iSubItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return 0;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return 0;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];
	return pSubItem->m_dwItemData;
}

INT_PTR CSTXListControlEx::SetItemCaption( INT_PTR iItemIndex, LPCTSTR lpszCaption, BOOL bSubCaption)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	if(bSubCaption)
		pItem->m_strSubCaption = lpszCaption;
	else
		pItem->m_strCaption = lpszCaption;

	Invalidate();
	return iItemIndex;
}

void CSTXListControlEx::SetBackColor( COLORREF clrColor )
{
	m_vBackColorRed = GetRValue(clrColor) / 256.0;
	m_vBackColorGreen = GetGValue(clrColor) / 256.0;
	m_vBackColorBlue = GetBValue(clrColor) / 256.0;

	Invalidate();
}

void CSTXListControlEx::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	BOOL bInHeader = FALSE;
	INT_PTR iHit = ItemHitTest(point, &bInHeader);
	if(iHit != -1)
	{
		std::tr1::shared_ptr<CSTXListControlExItem> pHitNow = m_arrItems[iHit];
		INT_PTR iHitSubItem = SubItemHitTest(iHit, point);
		if(iHitSubItem != -1)
			pHitNow = m_arrItems[iHit]->m_arrChildItems[iHitSubItem];

		NMSTXALITEMDBLCLICK nm;
		nm.hdr.hwndFrom = GetSafeHwnd();
		nm.hdr.idFrom = GetDlgCtrlID();
		nm.hdr.code = STXALN_ITEMDBLCLICK;
		nm.iIndex = iHit;
		nm.bInHeader = bInHeader;
		nm.iSubIndex = iHitSubItem;
		nm.nMouseButton = 0;	//Left Button
		nm.dwItemData = pHitNow->m_dwItemData;

		LRESULT result = GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm);
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CSTXListControlEx::OnUpKeyDown()
{
	FocusPreviousItem();
}

void CSTXListControlEx::OnDownKeyDown()
{
	FocusNextItem();
}


BOOL CSTXListControlEx::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
		case VK_UP:
			OnUpKeyDown();
			return TRUE;
			break;
		case VK_DOWN:
			OnDownKeyDown();
			return TRUE;
			break;
		case VK_SPACE:
			OnSpaceKeyDown();
			return TRUE;
			break;
		case VK_LEFT:
			OnLeftKeyDown();
			return TRUE;
			break;
		case VK_RIGHT:
			OnRightKeyDown();
			return TRUE;
			break;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void CSTXListControlEx::FocusPreviousItem()
{
	if(m_pFocusedItem == NULL)
	{
		if(m_arrItems.GetCount() > 0)
		{
			SetFocusedItem(0, -1);
		}
	}
	else
	{
		if(m_pFocusedItem->m_pParentItem != NULL)	// SubItem
		{
			if(m_iFocusSubItemIndex >= 0)
				SetFocusedItem(m_iFocusItemIndex, m_iFocusSubItemIndex - 1);
		}
		else
		{
			if(m_iFocusItemIndex > 0)
			{
				int nChildrenCount = m_arrItems[m_iFocusItemIndex -1]->m_arrChildItems.GetCount();

				if(!m_arrItems[m_iFocusItemIndex -1]->m_bExpand)
					nChildrenCount = 0;

				SetFocusedItem(m_iFocusItemIndex - 1, nChildrenCount - 1);
			}
		}
	}
}

void CSTXListControlEx::FocusNextItem()
{
	if(m_pFocusedItem == NULL)
	{
		if(m_arrItems.GetCount() > 0)
		{
			SetFocusedItem(0, -1);
		}
	}
	else
	{
		if(m_pFocusedItem->m_pParentItem != NULL)	// SubItem
		{
			if(m_iFocusSubItemIndex < m_pFocusedItem->m_pParentItem->m_arrChildItems.GetCount() - 1)
				SetFocusedItem(m_iFocusItemIndex, m_iFocusSubItemIndex + 1);
			else
			{
				if(m_iFocusItemIndex < m_arrItems.GetCount() - 1)
					SetFocusedItem(m_iFocusItemIndex + 1, -1);
			}
		}
		else
		{
			int nChildrenCount = m_pFocusedItem->m_arrChildItems.GetCount();
			if(!m_pFocusedItem->m_bExpand)
				nChildrenCount = 0;

			if(nChildrenCount > 0)
			{
				SetFocusedItem(m_iFocusItemIndex, 0);
			}
			else
			{
				if(m_iFocusItemIndex < m_arrItems.GetCount() - 1)
					SetFocusedItem(m_iFocusItemIndex + 1, -1);
			}
		}
	}
}


BOOL CSTXListControlEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if((GetStyle() & WS_VSCROLL) == WS_VSCROLL)
	{
		//bVScrollExist = TRUE;
	}
	else
	{
		//bVScrollExist = FALSE;
		return CWnd::OnMouseWheel(nFlags, zDelta, pt);
	}

	int minpos;
	int maxpos;
	GetScrollRange(SB_VERT, &minpos, &maxpos); 
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(SB_VERT);
	int newpos = curpos - zDelta;
	newpos = min(newpos, maxpos);
	newpos = max(newpos, minpos);

	SetScrollPos(SB_VERT, newpos);
	Invalidate();

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CSTXListControlEx::OnSpaceKeyDown()
{
	if(m_pFocusedItem && m_pFocusedItem->m_pParentItem == NULL)
	{
		ExpandItem(m_iFocusItemIndex, !m_pFocusedItem->m_bExpand);
	}
}

void CSTXListControlEx::InternalEnsureVisible( tr1::shared_ptr<CSTXListControlExItem> pItemEx )
{
	if(pItemEx == NULL)
		return;

	int iOffsetLow = 0;
	int iOffsetHigh = 0;
	if(pItemEx->m_pParentItem == NULL)
	{
		iOffsetLow = (int)pItemEx->m_iOffset.GetValue();
		iOffsetHigh = (int)iOffsetLow + pItemEx->GetHeaderHeight();
	}
	else
	{
		iOffsetLow = (int)(pItemEx->m_pParentItem->m_iOffset.GetValue() + pItemEx->m_iOffset.GetValue());
		iOffsetHigh = iOffsetLow + DEFAULT_ITEM_HEIGHT;
	}
	CRect rcClient;
	GetClientRect(rcClient);
	int iScrollPos = GetScrollPos(SB_VERT);
	int iViewMin = iScrollPos;
	int iViewMax = iScrollPos + rcClient.Height();

	if(iOffsetLow < iViewMin)
	{
		SetScrollPos(SB_VERT, max(0, iOffsetLow));
	}
	else if(iOffsetHigh > iViewMax)
	{
		int nScMin = 0;
		int nScMax = 0;
		GetScrollRange(SB_VERT, &nScMin, &nScMax);
		SetScrollPos(SB_VERT, min(nScMax, iOffsetHigh - rcClient.Height()));
	}
}

void CSTXListControlEx::OnLeftKeyDown()
{
	if(m_pFocusedItem && m_pFocusedItem->m_pParentItem == NULL)
	{
		ExpandItem(m_iFocusItemIndex, FALSE);
	}
}

void CSTXListControlEx::OnRightKeyDown()
{
	if(m_pFocusedItem && m_pFocusedItem->m_pParentItem == NULL)
	{
		ExpandItem(m_iFocusItemIndex, TRUE);
	}
}

INT_PTR CSTXListControlEx::GetSubItemCount( INT_PTR iItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return 0;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	return pItem->m_arrChildItems.GetCount();
}

COLORREF CSTXListControlEx::GetBackgroundColor()
{
	return RGB((int)(m_vBackColorRed.GetValue()*255),(int)(m_vBackColorGreen.GetValue()*255),(int)(m_vBackColorBlue.GetValue()*255));
}
/*
void CALLBACK CSTXListControlEx::OnVariableStatusChange(DWORD_PTR dwUserData, BOOL bActive)
{
	CSTXListControlExItem *pThis = (CSTXListControlExItem*)dwUserData;
	if(bActive)
		pThis->IncreaseActivate();
	else
		pThis->DecreaseActivate();
}

void CSTXListControlEx::IncreaseActivate()
{
	if(m_nActivateCount == 0)
	{
		OutputDebugString(_T("ActivateAnimation\n"));
		ActivateAnimation();
	}
	m_nActivateCount++;
}

void CSTXListControlEx::DecreaseActivate()
{
	m_nActivateCount--;
	if(m_nActivateCount == 0)
	{
		OutputDebugString(_T("DeactivateAnimation\n"));
		DeactivateAnimation();
	}
}
*/

void CSTXListControlEx::SetGlobalUseGrayImage(BOOL bGray)
{
	m_bGlobalUseGrayImage = bGray;
	Invalidate();
}

void CSTXListControlEx::OnBeginDrag(std::tr1::shared_ptr<CSTXListControlExItem> pDragItem)
{
	//Do something in derived class
}

void CSTXListControlEx::SetEmptyText( LPCTSTR lpszText )
{
	m_strTextWhenEmpty = lpszText;
	Invalidate();
}

INT_PTR CSTXListControlEx::SetItemTag1(INT_PTR iItemIndex, LPCTSTR lpszTag)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];
	pItem->m_strTag1 = lpszTag;

	return iItemIndex;
}

INT_PTR CSTXListControlEx::FindItemWithTag1(LPCTSTR lpszTag, BOOL bCaseSensitive)
{
	if(bCaseSensitive)
	{
		for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
		{
			std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			if(_tcscmp(pItem->m_strTag1, lpszTag) == 0)
			{
				return i;
			}
		}
	}
	else
	{
		for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
		{
			std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
			if(_tcsicmp(pItem->m_strTag1, lpszTag) == 0)
			{
				return i;
			}
		}
	}

	return -1;
}

INT_PTR CSTXListControlEx::DeleteAllSubItems( INT_PTR iItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	INT_PTR nSubItems = pItem->GetSubItemCount();

	//Adjust all the offsets after this item
	for(INT_PTR i = nSubItems - 1;i>= 0;i--)
	{
		DeleteSubItem(iItemIndex, i);
	}

	return iItemIndex;
}

INT_PTR CSTXListControlEx::SetItemCaptionColor( INT_PTR iItemIndex, COLORREF clrColor )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	std::tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	pItem->m_clrMainCaptionColor = clrColor;

	if(GetSafeHwnd())
		Invalidate();

	return iItemIndex;
}

INT_PTR CSTXListControlEx::SetSubItemCaptionColor( INT_PTR iItemIndex, INT_PTR iSubItemIndex, COLORREF clrColor )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	pSubItem->m_clrMainCaptionColor = clrColor;

	if(GetSafeHwnd())
		Invalidate();

	return iSubItemIndex;
}

INT_PTR CSTXListControlEx::SetSubItemSubCaptionColor( INT_PTR iItemIndex, INT_PTR iSubItemIndex, COLORREF clrColor )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	pSubItem->m_clrSubCaptionColor = clrColor;

	if(GetSafeHwnd())
		Invalidate();

	return iSubItemIndex;
}

INT_PTR CSTXListControlEx::ExcludeItemFromFindSubItem( INT_PTR iItemIndex, BOOL bExclude )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	pItem->m_bExcludeFromFindSubItem = bExclude;
	return iItemIndex;
}

INT_PTR CSTXListControlEx::SetSubImage( INT_PTR iImageIndex, IStream *pStream )
{
	if(m_arrSubMarkImages.GetUpperBound() < iImageIndex)
		m_arrSubMarkImages.SetSize(iImageIndex + 1);

	m_arrSubMarkImages[iImageIndex] = GetResizedImage(pStream, DEFAULT_ITEM_HEIGHT / 2);
	return iImageIndex;
}

INT_PTR CSTXListControlEx::SetSubImage( INT_PTR iImageIndex, HBITMAP hBitmap )
{
	if(m_arrSubMarkImages.GetUpperBound() < iImageIndex)
		m_arrSubMarkImages.SetSize(iImageIndex + 1);

	m_arrSubMarkImages[iImageIndex] = GetResizedImage(hBitmap, DEFAULT_ITEM_HEIGHT / 2);
	return iImageIndex;
}

INT_PTR CSTXListControlEx::SetSubImage( INT_PTR iImageIndex, LPCTSTR lpszFile )
{
	if(m_arrSubMarkImages.GetUpperBound() < iImageIndex)
		m_arrSubMarkImages.SetSize(iImageIndex + 1);

	m_arrSubMarkImages[iImageIndex] = GetResizedImage(lpszFile, DEFAULT_ITEM_HEIGHT / 2);
	return iImageIndex;
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXListControlEx::GetResizedImage( IStream *pStream, int nWidthHeight )
{
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(pStream));
	return GetResizedImage(img, nWidthHeight);
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXListControlEx::GetResizedImage( HBITMAP hBitmap, int nWidthHeight )
{
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	return GetResizedImage(img, nWidthHeight);
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXListControlEx::GetResizedImage( LPCTSTR lpszFile, int nWidthHeight )
{
	tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(lpszFile));
	return GetResizedImage(img, nWidthHeight);
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXListControlEx::GetResizedImage( std::tr1::shared_ptr<Gdiplus::Image> pImage, int nWidthHeight )
{
	UINT o_height = pImage->GetHeight();
	UINT o_width = pImage->GetWidth();

	if(nWidthHeight > 0 && (o_height > nWidthHeight || o_width > nWidthHeight))
	{
		INT n_width = nWidthHeight;
		INT n_height = nWidthHeight;
		double ratio = ((double)o_width) / ((double)o_height);
		if (o_width > o_height) 
		{
			// Resize down by width
			n_height = static_cast<UINT>(((double)n_width) / ratio);
		} 
		else
		{
			n_width = static_cast<UINT>(n_height * ratio);
		}

		tr1::shared_ptr<Gdiplus::Image> imgThumbnail(pImage->GetThumbnailImage(n_width, n_height));
		return imgThumbnail;
	}
	else
	{
		return pImage;
	}
}

INT_PTR CSTXListControlEx::SetSubItemSubImageIndex(INT_PTR iItemIndex, INT_PTR iSubItemIndex, INT_PTR iImageIndex)
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	ASSERT(iImageIndex <= m_arrSubMarkImages.GetUpperBound());
	pSubItem->m_iMarkImage = iImageIndex;
	return iSubItemIndex;
}

UINT CSTXListControlEx::GetTopReservedHeight()
{
	return 0;
}

void CSTXListControlEx::DrawTopReservedSpace( Gdiplus::Graphics *pGraphics )
{
	//Draw in derived class
}

INT_PTR CSTXListControlEx::AddItemButton( INT_PTR iItemIndex, IStream *pStreamImage, LPCTSTR lpszToolTips )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	tr1::shared_ptr<CSTXListControlExButton> pBtn(new CSTXListControlExButton(pItem.get(), pStreamImage, lpszToolTips));
	pBtn->m_iOffset = -DEFAULT_BUTTON_WIDTH;

	//Adjust offset of existing buttons
	for(INT_PTR i=0;i<pItem->m_arrButtons.GetCount();i++)
	{
		DOUBLE nFinalOffset = pItem->m_arrButtons[i]->m_iOffset.GetFinalValue();
		pItem->m_arrButtons[i]->m_iOffset = nFinalOffset - DEFAULT_BUTTON_WIDTH;
	}

	pItem->m_arrButtons.Add(pBtn);

	pBtn->m_iOffset.GetValue();

	return iItemIndex;
}


INT_PTR CSTXListControlEx::AddSubItemButton( INT_PTR iItemIndex, INT_PTR iSubItemIndex, IStream *pStreamImage, LPCTSTR lpszToolTips )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return -1;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	tr1::shared_ptr<CSTXListControlExButton> pBtn(new CSTXListControlExButton(pSubItem.get(), pStreamImage, lpszToolTips));
	pBtn->m_iOffset = -DEFAULT_BUTTON_WIDTH;

	//Adjust offset of existing buttons
	for(INT_PTR i=0;i<pSubItem->m_arrButtons.GetCount();i++)
	{
		DOUBLE nFinalOffset = pSubItem->m_arrButtons[i]->m_iOffset.GetFinalValue();
		pSubItem->m_arrButtons[i]->m_iOffset = nFinalOffset - DEFAULT_BUTTON_WIDTH;
	}

	pSubItem->m_arrButtons.Add(pBtn);

	pBtn->m_iOffset.GetValue();

	return iSubItemIndex;
}

CString CSTXListControlEx::GetItemTag1( INT_PTR iItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return _T("");

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	return pItem->m_strTag1;
}

CString CSTXListControlEx::GetItemTag2( INT_PTR iItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return _T("");

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	return pItem->m_strTag2;
}

INT_PTR CSTXListControlEx::GetSubItemButtonCount( INT_PTR iItemIndex, INT_PTR iSubItemIndex )
{
	if(iItemIndex < 0 || iItemIndex > m_arrItems.GetCount() - 1)
		return 0;

	tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[iItemIndex];

	if(iSubItemIndex < 0 || iSubItemIndex > pItem->m_arrChildItems.GetCount() - 1)
		return 0;

	tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[iSubItemIndex];

	return pSubItem->m_arrButtons.GetCount();
}

void CSTXListControlEx::DrawBackground( Gdiplus::Graphics *pGraphics )
{
	CRect rcThis;
	GetClientRect(rcThis);

	if(m_pImgBackground)
	{
		if(!m_pImgBackgroundCached)
		{
			Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(rcThis.Width(), rcThis.Height()); 
			Gdiplus::Graphics graphics(pBitmap);
			graphics.DrawImage(m_pImgBackground.get(), 0, 0, rcThis.Width(), rcThis.Height());

			Gdiplus::Bitmap *pBitmapSrc = pBitmap;
			tr1::shared_ptr<Gdiplus::CachedBitmap> imgCached(new Gdiplus::CachedBitmap(pBitmapSrc, pGraphics));
			m_pImgBackgroundCached = imgCached;

			delete pBitmap;
		}
		if(m_pImgBackgroundCached)
			pGraphics->DrawCachedBitmap(m_pImgBackgroundCached.get(), 0, 0);
		else
			pGraphics->DrawImage(m_pImgBackground.get(), rcThis.left, rcThis.top, rcThis.Width(), rcThis.Height());
	}
	else
	{
		Gdiplus::SolidBrush brushBk(Gdiplus::Color(
			(FLOAT)m_vBackColorRed.GetValue() * 255,
			(FLOAT)m_vBackColorGreen.GetValue() * 255,
			(FLOAT)m_vBackColorBlue.GetValue() * 255));
		pGraphics->FillRectangle(&brushBk, rcThis.left, rcThis.top, rcThis.Width(), rcThis.Height());
	}
}

INT_PTR CSTXListControlEx::SetCurSel( INT_PTR iItemIndex, INT_PTR iSubItemIndex )
{
	SetFocusedItem(iItemIndex, iSubItemIndex);
	return iItemIndex;
}

void CSTXListControlEx::SetSubItemLeftPadding( int nPadding )
{
	m_vSubItemLeftSpacing = nPadding;

	DOUBLE fPadding = m_vSubItemLeftSpacing.GetValue();		//Activated
	for(INT_PTR i=0;i<m_arrItems.GetCount();i++)
	{
		tr1::shared_ptr<CSTXListControlExItem> pItem = m_arrItems[i];
		for(INT_PTR k=0;k<pItem->m_arrChildItems.GetCount();k++)
		{
			tr1::shared_ptr<CSTXListControlExItem> pSubItem = pItem->m_arrChildItems[k];
			pSubItem->m_iLeftPadding = fPadding;
		}
	}
}