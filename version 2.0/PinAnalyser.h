
// PinAnalyser.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CPinAnalyserApp:
// Сведения о реализации этого класса: PinAnalyser.cpp
//

class CPinAnalyserApp : public CWinApp
{
public:
	CPinAnalyserApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CPinAnalyserApp theApp;
