// ColorListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ColorListCtrl.h"


// CColorListCtrl

IMPLEMENT_DYNAMIC(CColorListCtrl, CListCtrl)

CColorListCtrl::CColorListCtrl()
{
	m_GridMode = FALSE;
}

CColorListCtrl::~CColorListCtrl()
{
}

BEGIN_MESSAGE_MAP(CColorListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteItem)
	ON_NOTIFY_REFLECT(LVN_DELETEALLITEMS, OnDeleteAllItems)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, CustomizeDraw)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnInsertItem)
END_MESSAGE_MAP()

// CColorListCtrl message handlers

void CColorListCtrl::CustomizeDraw(NMHDR *pNotifyStruct, LRESULT *result)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW) pNotifyStruct;

	long nRow = 0, nCol = 0;
	long nColIdx = 0, nCellIdx = 0, nRowIdx = 0;
	
	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*result  = CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYSUBITEMDRAW;	
		break;
	case CDDS_ITEMPREPAINT:
		*result = CDRF_NOTIFYSUBITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT|CDDS_SUBITEM:	
		nRow = lplvcd->nmcd.dwItemSpec;	
		nCol = lplvcd->iSubItem;

		if(m_GridMode)
		{
			if(nRow %2 == 1)
			{
				if(nCol %2 == 0)
					lplvcd->clrTextBk = m_ColorGridX;
				else
					lplvcd->clrTextBk = m_ColorGridY;
			}else
			{
				if(nCol %2 == 0)
					lplvcd->clrTextBk = m_ColorGridY;
				else
					lplvcd->clrTextBk = m_ColorGridX;
			}
		}

		//if(AltRowMode)	
		//{
		//	if( iRow %2 == 1)
		//		lplvcd->clrTextBk = Rx;
		//	else
		//		lplvcd->clrTextBk = Ry;
		//}

		nRowIdx = FindColorRecord(nRow, RECORD_ROW);	
		nColIdx = FindColorRecord(nCol, RECORD_COL);
		nCellIdx = FindColorRecord(nRow, RECORD_CELL, nCol);

		if(nRowIdx == -1 && nColIdx == -1 && nCellIdx == -1 )
		{
			if(!m_GridMode/* && !AltRowMode*/)
			{
				lplvcd->clrTextBk = GetBkColor();
			}
		}else
		{
			if(nCellIdx != -1)
			{
				lplvcd->clrTextBk = CellColorRecord[nCellIdx].itemBGColor;
				lplvcd->clrText = CellColorRecord[nCellIdx].itemTextColor;
			}else
			{
				if(nRowIdx != -1)
				{
					lplvcd->clrTextBk = RowColorRecord[nRowIdx].itemBGColor;
					lplvcd->clrText = RowColorRecord[nRowIdx].itemTextColor;
				}else
				{
					lplvcd->clrTextBk = ColColorRecord[nColIdx].itemBGColor;
					lplvcd->clrText = ColColorRecord[nColIdx].itemTextColor;
				}
			}
		}

		/*if(MaskMode)		
			lplvcd->clrText = InvertColor(lplvcd->clrTextBk);*/

		*result = CDRF_NEWFONT;
		break;

	default:
		*result = CDRF_DODEFAULT;
	}

}

void CColorListCtrl::OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult) 
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

	long nRow  = pNMListView->iItem;
	long nColorIdx = FindColorRecord(nRow);

	for(int i=0 ; i<(int)RowColorRecord.size() ; i++)
	{
		if(RowColorRecord[i].itemID == nRow)
		{
			RowColorRecord.erase(RowColorRecord.begin()+i);
			i--;
			continue;
		}
		if(RowColorRecord[i].itemID > nRow)
		{
			RowColorRecord[i].itemID--;
		}
	}

	for(int i=0 ; i<(int)CellColorRecord.size() ; i++)
	{
		if(CellColorRecord[i].itemID == nRow)
		{
			CellColorRecord.erase(CellColorRecord.begin()+i);
			i--;
			continue;
		}
		if(CellColorRecord[i].itemID > nRow)
		{
			CellColorRecord[i].itemID--;
		}
	}
	*pResult = 0;
}

void CColorListCtrl::OnDeleteAllItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	RowColorRecord.clear();
	ColColorRecord.clear();
	CellColorRecord.clear();

	*pResult = 0;
}

void CColorListCtrl::OnInsertItem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	long nRow = pNMListView->iItem;
	long nCol = pNMListView->iSubItem;

	for(int i=0 ; i<(int)RowColorRecord.size() ; i++)
	{
		if(RowColorRecord[i].itemID > nRow)
		{
			RowColorRecord[i].itemID++;
		}
	}

	for(int i=0 ; i<(int)CellColorRecord.size() ; i++)
	{
		if(CellColorRecord[i].itemID > nRow)
		{
			CellColorRecord[i].itemID++;
		}
	}

	*pResult = 0;
}

void CColorListCtrl::SetRowColor(long nRow, COLORREF cfTextColor, COLORREF cfBGColor)
{
	COLORSTRUCT ColorStruct;
	ColorStruct.itemBGColor = cfBGColor ? cfBGColor : GetBkColor();
	ColorStruct.itemTextColor = cfTextColor;
	ColorStruct.itemID = nRow;
	RowColorRecord.push_back(ColorStruct);

	RedrawWindow(NULL,NULL,RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CColorListCtrl::SetColColor(long nCol, COLORREF cfTextColor, COLORREF cfBGColor)
{
	COLORSTRUCT ColorStruct;
	ColorStruct.itemBGColor =cfBGColor ? cfBGColor : GetBkColor();
	ColorStruct.itemTextColor = cfTextColor;
	ColorStruct.itemID = nCol;
	ColColorRecord.push_back(ColorStruct);

	RedrawWindow(NULL,NULL,RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CColorListCtrl::SetCellColor(long nRow, long nCol, COLORREF cfTextColor, COLORREF cfBGColor)
{
	COLORSTRUCT ColorStruct;
	ColorStruct.itemBGColor = cfBGColor ? cfBGColor : GetBkColor();
	ColorStruct.itemTextColor = cfTextColor;
	ColorStruct.itemID = nRow;
	ColorStruct.itemSubID = nCol;
	CellColorRecord.push_back(ColorStruct);

	RedrawWindow(NULL,NULL,RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

long CColorListCtrl::FindColorRecord(long itemID, RECORD_TYPE RecordType, long itemSubID)
{
	long nColorRecord = -1;

	if(RecordType == RECORD_ROW)
	{
		for(int i=0 ; i<(int)RowColorRecord.size() ; i++)
		{
			if(RowColorRecord[i].itemID == itemID)
			{
				nColorRecord = i;
				goto go_End;
			}
		}
	}

	if(RecordType == RECORD_COL)
	{
		for(int i=0 ; i<(int)ColColorRecord.size() ; i++)
		{
			if(ColColorRecord[i].itemID == itemID)
			{
				nColorRecord = i;
				goto go_End;
			}
		}
	}

	if(RecordType == RECORD_CELL)
	{
		for(int i=0 ; i<(int)CellColorRecord.size(); i++)
		{
			if(itemSubID >= 0)
			{
				if(CellColorRecord[i].itemID == itemID && CellColorRecord[i].itemSubID == itemSubID)
				{
					nColorRecord = i;
					goto go_End;
				}
			}
		}
	}

go_End:
	return nColorRecord;
}