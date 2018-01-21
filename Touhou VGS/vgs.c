/* Copyright 2012, Suzuki Plan.
 *----------------------------------------------------------------------------
 * [�ď�] VGS(Vide Game Sound) Version 1.00
 * [�T�v] �Ǝ��\�t�g�E�F�A�E�V���Z�T�C�U�[ VG-Sound
 * [���l] �J�[�l����ʂɈˑ����Ȃ���������������(Windows/Android���p)
 *        �������AC�W���֐��͑S�ẴJ�[�l���Ŏg������̂Ƃ���.
 *----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "vge.h"
#include "vgeint.h"
#include "vgsdec.h"

int _bstop;
static inline void setNote(unsigned char cn, unsigned char t, unsigned char n);
inline int getNextNote();

/*
 *----------------------------------------------------------------------------
 * �T�E���h���̃o�b�t�@�����O
 *----------------------------------------------------------------------------
 */
void vgsbuf(char* buf, size_t size)
{
    static int an;
    int i, j;
    int wav;
    int cs;
    short* bp;

    an = 1 - an;

    memset(buf, 0, size);
    if (_pause || vge_getmute()) {
        return;
    }

    /* ���ʉ��̃o�b�t�@�����O */
    for (i = 0; i < MAXSLOT; i++) {
        if (_eff[i].flag) {
            if (1 < _eff[i].flag) {
                eff_pos(&_eff[i], 0);
                eff_flag(&_eff[i], 1);
            }
            /* �R�s�[�T�C�Y�̌v�Z */
            cs = _eff[i].size - _eff[i].pos;
            if (size < (size_t)cs) {
                cs = (int)size;
            }
            /* �o�b�t�@�����O */
            for (j = 0; j < cs; j += 2) {
                bp = (short*)(&buf[j]);
                wav = *bp;
                wav += *((short*)&(_eff[i].dat[_eff[i].pos + j]));
                if (32767 < wav)
                    wav = 32767;
                else if (wav < -32768)
                    wav = -32768;
                (*bp) = (short)wav;
            }
            /* �|�W�V�����E�`�F���W */
            eff_pos(&_eff[i], _eff[i].pos + cs);
            if (_eff[i].size <= _eff[i].pos) {
                /* �����I�� */
                eff_flag(&_eff[i], 0);
            }
        } else {
            eff_pos(&_eff[i], 0);
        }
    }

    if (_bstop) return;
    vgsdec_execute(_psg, buf, size);
}
