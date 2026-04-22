#pragma once
#pragma execution_character_set("utf-8")
#include "afxdialogex.h"
#include "resource.h"
#include <vector>
#include <string>
#include <unordered_set>

// Диалоговое окно CPinAnalyzerDlg
class CPinAnalyzerDlg : public CDialogEx
{
public:
    CPinAnalyzerDlg(CWnd* pParent = nullptr);   // стандартный конструктор

    // Данные диалогового окна
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_PINANALYZER_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedBtnLoad();
    afx_msg void OnBnClickedBtnAnalyze();

private:
    // Логика анализа
    bool isAllSame(const std::string& s);
    bool isSequential(const std::string& s);
    bool isRepeating(const std::string& s);
    bool isDate(const std::string& s);
    bool isKeyboardPattern(const std::string& s);
    std::unordered_set<std::string> getCommonPINs(int len);
    int calculateScore(const std::string& pin, int len);
    std::string getVerdict(int score, const std::string& pin, int len);
    void ParseAndAnalyze(const CString& text, int mode);

    // Переменные элементов управления
    CEdit   m_editPin;
    CListBox m_listGood;
    CListBox m_listMedium;
    CListBox m_listBad;
};