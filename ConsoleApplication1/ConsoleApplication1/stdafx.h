// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include "fglove.h"
#include "NIDAQmx.h"
#include <string.h>
#include <time.h>
#include <sys/utime.h>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#include <windows.h> // for Sleep

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

const int MAX_SAMPLE = 5000;