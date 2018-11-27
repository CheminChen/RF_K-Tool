#pragma once
#include <vector>
using namespace std;

// CColorListCtrl
typedef struct _COLORSTRCUT
{
	COLORREF itemBGColor;
	COLORREF itemTextColor;
	long itemID;
	long itemSubID;
}COLORSTRUCT, *PCOLORSTRUCT;

enum RECORD_TYPE
{
	RECORD_ROW = 0,
	RECORD_COL = 1,
	RECORD_CELL = 2
};

class CColorListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CColorListCtrl)

public:
	CColorListCtrl();
	virtual ~CColorListCtrl();

protected:
	afx_msg void OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeleteAllItems(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnInsertItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void CustomizeDraw( NMHDR *pNotifyStruct, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:
	void SetRowColor(long nRow, COLORREF cfTextColor, COLORREF cfBGColor=NULL);
	void SetColColor(long nCol, COLORREF cfTextColor, COLORREF cfBGColor=NULL);
	void SetCellColor(long nRow, long nCol, COLORREF cfTextColor, COLORREF cfBGColor=NULL);

protected:
	long FindColorRecord(long itemID, RECORD_TYPE RecordType=RECORD_ROW, long itemSubID=-1);

	vector<COLORSTRUCT> RowColorRecord;
	vector<COLORSTRUCT> ColColorRecord;
	vector<COLORSTRUCT> CellColorRecord;

	BOOL				m_GridMode;
	COLORREF			m_ColorGridX;
	COLORREF			m_ColorGridY;
};


