#pragma once


// CMyImage

class CMyImage : public CStatic
{
	DECLARE_DYNAMIC(CMyImage)

public:
	CMyImage();
	virtual ~CMyImage();

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

public:
	void SetMaskColor(COLORREF crMask);

private:
	COLORREF				m_crMask;
};


