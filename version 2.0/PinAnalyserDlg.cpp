#include "pch.h"
#include "framework.h"
#include "PinAnalyser.h"
#include "PinAnalyserDlg.h"
#include "afxdialogex.h"
#include "CString"
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CPinAnalyzerDlg::CPinAnalyzerDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_PINANALYSER_DIALOG, pParent) {}


void CPinAnalyzerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_EDIT_PIN, m_editPin);
    DDX_Control(pDX, IDC_LIST_GOOD, m_listGood);
    DDX_Control(pDX, IDC_LIST_MEDIUM, m_listMedium);
    DDX_Control(pDX, IDC_LIST_BAD, m_listBad);
}



BOOL CPinAnalyzerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // По умолчанию выбран 4-значный режим
    CheckRadioButton(IDC_RADIO_4, IDC_RADIO_6, IDC_RADIO_4);

    return TRUE;
}

BEGIN_MESSAGE_MAP(CPinAnalyzerDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_LOAD, &CPinAnalyzerDlg::OnBnClickedBtnLoad)
    ON_BN_CLICKED(IDC_BTN_ANALYZE, &CPinAnalyzerDlg::OnBnClickedBtnAnalyze)
    ON_BN_CLICKED(IDC_BTN_SAVE, &CPinAnalyzerDlg::OnBnClickedBtnSave)
END_MESSAGE_MAP()





bool CPinAnalyzerDlg::isAllSame(const std::string& s)
{
    if (s.empty()) return false;
    char first = s[0];
    return std::all_of(s.begin() + 1, s.end(), [first](char c) { return c == first; });
}

bool CPinAnalyzerDlg::isSequential(const std::string& s)
{
    if (s.size() < 2) return false;
    bool asc = true, desc = true;
    for (size_t i = 1; i < s.size(); ++i) {
        if (s[i] != s[i - 1] + 1) asc = false;
        if (s[i] != s[i - 1] - 1) desc = false;
    }
    return asc || desc;
}

bool CPinAnalyzerDlg::isRepeating(const std::string& s)
{
    if (s.size() < 4) return false;
    if (s.size() == 4) {
        if (s[0] == s[1] && s[2] == s[3]) return true;
        if (s[0] == s[2] && s[1] == s[3]) return true;
        if (s[0] == s[3] && s[1] == s[2]) return true;
    }
    if (s.size() == 6) {
        if (s[0] == s[1] && s[2] == s[3] && s[4] == s[5]) return true;
        if (s[0] == s[2] && s[1] == s[3] && s[4] == s[5]) return true;
    }
    int consecutive = 0;
    for (size_t i = 1; i < s.size(); ++i) {
        if (s[i] == s[i - 1]) consecutive++;
        else consecutive = 0;
        if (consecutive >= 2) return true;
    }
    return false;
}

bool CPinAnalyzerDlg::isDate(const std::string& s)
{
    if (s.size() != 4) return false;
    int month = (s[0] - '0') * 10 + (s[1] - '0');
    int day = (s[2] - '0') * 10 + (s[3] - '0');
    return (month >= 1 && month <= 12) && (day >= 1 && day <= 31);
}

bool CPinAnalyzerDlg::isKeyboardPattern(const std::string& s)
{
    static const std::unordered_set<std::string> kb4 = { "2580","0852","1470","0369","1590","7531","8642","9753","0258" };
    static const std::unordered_set<std::string> kb6 = { "14702580","25801470" };
    if (s.size() == 4) return kb4.count(s);
    if (s.size() == 6) return kb6.count(s);
    return false;
}

std::unordered_set<std::string> CPinAnalyzerDlg::getCommonPINs(int len)
{
    if (len == 4) {
        return { "1234","1111","0000","1212","7777","1004","2000","4444","2222","6969","9999","1997","2007","1999","1941","2011","2012","2017","2018","2024","2025",
                 "3333","5555","6666","1122","1313","8888","4321","2001","1010","1984","2003","2005","2008","1987","1945","2013","2014","2019","2021","2026","2027",
                 "2010","2020","2580","1357","2468","1590","6767","5252","4242","2002","2004","2006","2009","1876","1998","2015","2016","2022","2023","2028","2029" };
    }
    else {
        return { "123456","111111","123123","000000","123321","654321","666666","121212","232323","343434","454545","565656","676767","787878","898989","909090"
                 "112233","555555","222222","333333","444444","777777","888888","999999","010101","101010","000111","111000" };
    }
}

int CPinAnalyzerDlg::calculateScore(const std::string& pin, int len)
{
    if (pin.length() != static_cast<size_t>(len)) return 0;
    if (getCommonPINs(len).count(pin)) return 1;

    int score = 10;
    if (isAllSame(pin)) score -= 6;
    if (isSequential(pin)) score -= 5;
    if (isRepeating(pin)) score -= 3;
    if (isKeyboardPattern(pin)) score -= 3;
    if (isDate(pin)) score -= 2;

    bool hasEven = false, hasOdd = false;
    for (char c : pin) {
        int d = c - '0';
        if (d % 2 == 0) hasEven = true;
        else hasOdd = true;
    }
    if (hasEven && hasOdd) score += 1;

    if (score < 1) score = 1;
    if (score > 10) score = 10;
    return score;
}

std::string CPinAnalyzerDlg::getVerdict(int score, const std::string& pin, int len)
{
    std::string verdict = "Grade: " + std::to_string(score) + "/10\n";
    if (score == 10) {
        verdict += " excellent";
    }
    else if (score == 9) {
        verdict += " great";
    }
    else if (score == 8) {
        verdict += " quite alright";
    }
    else if (score == 7) {
        verdict += " good but could be a pattern";
    }
    else if (score == 6) {
        verdict += " mediocre and close to being easily guessable";
    }
    else {
        verdict += " unsecure";
        bool hasReason = false;
        if (isAllSame(pin) || isRepeating(pin)) {
            verdict += ", has repeating numbers";
            hasReason = true;
        }
        if (isSequential(pin)) {
            verdict += (hasReason ? " &" : ", as") + std::string(" it has ");
            hasReason = true;
        }
        if (getCommonPINs(len).count(pin)) {
            verdict += (hasReason ? " &" : ", as") + std::string(" it is in 1% most common combinations");
            hasReason = true;
        }
        if (isDate(pin)) {
            verdict += (hasReason ? " &" : ", as") + std::string(" it looks like a date");
            hasReason = true;
        }
        if (!hasReason) verdict += ", as it is too simple";
        verdict += ".";
    }
    return verdict;
}

// тут начинаются функции кнопочек

void CPinAnalyzerDlg::OnBnClickedBtnLoad()
{
    CFileDialog dlg(TRUE, _T("txt"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Текстовые файлы (*.txt)|*.txt|Все файлы (*.*)|*.*||"), this);

    if (dlg.DoModal() == IDOK)
    {
        std::ifstream file(CT2A(dlg.GetPathName()));
        if (!file.is_open()) {
            AfxMessageBox(_T("Не удалось открыть файл!"), MB_OK | MB_ICONERROR);
            return;
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        m_editPin.SetWindowText(CString(content.c_str()));
    }
}

void CPinAnalyzerDlg::OnBnClickedBtnAnalyze()
{
    // ресетаем списки при анаизе
    m_listGood.ResetContent();
    m_listMedium.ResetContent();
    m_listBad.ResetContent();

    int mode = IsDlgButtonChecked(IDC_RADIO_4) ? 4 : 6;

    CString input;
    m_editPin.GetWindowText(input);

    if (input.IsEmpty()) {
        AfxMessageBox(_T("Введите хотя бы один PIN-код!"), MB_OK | MB_ICONINFORMATION);
        return;
    }

    ParseAndAnalyze(input, mode);
}

void CPinAnalyzerDlg::ParseAndAnalyze(const CString& text, int mode)
{
    std::string str = CT2A(text);

    std::vector<std::string> pins;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t start = token.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        size_t end = token.find_last_not_of(" \t");
        token = token.substr(start, end - start + 1);
        if (!token.empty()) pins.push_back(token);
    }

    if (pins.empty()) {
        AfxMessageBox(_T("Не удалось разобрать PIN-коды"), MB_OK | MB_ICONERROR);
        return;
    }

    // а вот тут мы проверяем длину и что все цифры
    bool allValid = true;
    for (const auto& p : pins) {
        if (p.length() != static_cast<size_t>(mode) || 
            !std::all_of(p.begin(), p.end(), ::isdigit)) {
            allValid = false;
            break;
        }
    }

    if (!allValid) {
        AfxMessageBox(_T("Невозможно начать анализ.\nПросьба убедиться, что все комбинации чисел\nпо длине соответствуют выбранному варианту анализа!!"), MB_OK | MB_ICONERROR);
        return;
    }




    for (const auto& p : pins) {
        int score = calculateScore(p, mode);
        std::string verdict = getVerdict(score, p, mode);
        CString line = CString(p.c_str()) + _T(" - ") + CString(verdict.c_str());

        if (score >= 9)
            m_listGood.AddString(line);
        else if (score >= 6)
            m_listMedium.AddString(line);
        else
            m_listBad.AddString(line);
    }

    if (m_listGood.GetCount() == 0)   m_listGood.AddString(_T("Нет"));
    if (m_listMedium.GetCount() == 0) m_listMedium.AddString(_T("Нет"));
    if (m_listBad.GetCount() == 0)    m_listBad.AddString(_T("Нет"));
}

void CPinAnalyzerDlg::OnBnClickedBtnSave()
{
    if (m_listGood.GetCount() == 0 && m_listMedium.GetCount() == 0 && m_listBad.GetCount() == 0) {
        AfxMessageBox(_T("Нет данных для сохранения!"), MB_OK | MB_ICONQUESTION);
        return;
    }

    CFileDialog dlg(FALSE, _T("txt"), _T("results"),
        OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
        _T("Текстовые файлы (*.txt)|*.txt|Все файлы (*.*)|*.*||"), this);

    if (dlg.DoModal() != IDOK)
        return;

    CString filePath = dlg.GetPathName();

    std::ofstream file(CT2A(filePath), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        AfxMessageBox(_T("Не удалось создать файл!"), MB_OK | MB_ICONERROR);
        return;
    }

    file << "\xEF\xBB\xBF";
    file << "-+-+-+- РЕЗУЛЬТАТЫ АНАЛИЗА PIN-КОДОВ -+-+-+-\n\n";

    auto WriteList = [&](const CString& title, CListBox& listBox) {
        file << CT2A(title) << "\n";
        if (listBox.GetCount() > 0) {
            CString firstItem;
            listBox.GetText(0, firstItem);
            if (firstItem == _T("Нет") && listBox.GetCount() == 1) {
                file << "Нет\n";
            }
            else {
                for (int i = 0; i < listBox.GetCount(); ++i) {
                    CString str;
                    listBox.GetText(i, str);
                    file << CT2A(str) << "\n";
                }
            }
        }
        file << "\n";
        };

    WriteList(_T("ХОРОШИЕ (9-10/10):"), m_listGood);
    WriteList(_T("СРЕДНИЕ (6-8/10):"), m_listMedium);
    WriteList(_T("ПЛОХИЕ (1-5/10):"), m_listBad);

    file << "-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n";
    file << "Анализ выполнен: " << __DATE__ << " " << __TIME__ << "\n";

    file.close();

    AfxMessageBox(_T("Результаты успешно сохранены!"), MB_OK | MB_ICONINFORMATION);
}
