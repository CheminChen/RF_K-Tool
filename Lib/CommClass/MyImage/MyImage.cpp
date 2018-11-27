// MyImage.cpp : implementation file
//

#include "stdafx.h"
#include "MyImage.h"

// CMyImage

IMPLEMENT_DYNAMIC(CMyImage, CStatic)

CMyImage::CMyImage()
{
	m_crMask = RGB(0, 255, 0);
}

CMyImage::~CMyImage()
{
}

BEGIN_MESSAGE_MAP(CMyImage, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CMyImage message handlers

void CMyImage::SetMaskColor(COLORREF crMask)
{
	m_crMask = crMask;
}

void CMyImage::OnPaint()
{
	HBITMAP hbmpBitmap = GetBitmap();
	if(hbmpBitmap == NULL)
	{
		Default();
		return;
	}

	CPaintDC PaintDC(this);
	CDC MaskDC, MemoryDC;
	CBitmap MaskBitmap, *pOldMaskBitmap = NULL, *pOldMemoryBitmap = NULL;
	CRect rcClient;
	COLORREF crOldBack;

	GetClientRect(&rcClient);
	
	MaskDC.CreateCompatibleDC(&PaintDC);	
	MaskBitmap.CreateBitmap(rcClient.Width(), rcClient.Height(), 1, 1, NULL);
	pOldMaskBitmap = MaskDC.SelectObject(&MaskBitmap);	
	MemoryDC.CreateCompatibleDC(&PaintDC);
	pOldMemoryBitmap = MemoryDC.SelectObject(CBitmap::FromHandle(hbmpBitmap));

	crOldBack = MemoryDC.SetBkColor(m_crMask);

	MaskDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &MemoryDC, 0, 0, SRCCOPY);
	PaintDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &MemoryDC, 0, 0, SRCINVERT);
	PaintDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &MaskDC, 0, 0, SRCAND);
	PaintDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &MemoryDC, 0, 0, SRCINVERT);

	MemoryDC.SelectObject(pOldMaskBitmap);
	MaskDC.SelectObject(pOldMaskBitmap);

}
