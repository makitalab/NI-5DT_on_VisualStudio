// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
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

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă�������
#include <windows.h> // for Sleep

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

const int MAX_SAMPLE = 5000;