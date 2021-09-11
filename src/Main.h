/*
 *  Copyright (C) 2007-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
xmms-projectM v0.99 - xmms-projectm.sourceforge.net
--------------------------------------------------

Lead Developers:  Carmelo Piccione (cep@andrew.cmu.edu) &
                  Peter Sperl (peter@sperl.com)

We have also been advised by some professors at CMU, namely Roger B. Dannenberg.
http://www-2.cs.cmu.edu/~rbd/

The inspiration for this program was Milkdrop by Ryan Geiss. Obviously.

This code is distributed under the GPL.


THANKS FOR THE CODE!!!
-------------------------------------------------
The base for this program was andy@nobugs.org's XMMS plugin tutorial
http://www.xmms.org/docs/vis-plugin.html

We used some FFT code by Takuya OOURA instead of XMMS' built-in fft code
fftsg.c - http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html

For font rendering we used GLF by Roman Podobedov
glf.c - http://astronomy.swin.edu.au/~pbourke/opengl/glf/

and some beat detection code was inspired by Frederic Patin @
www.gamedev.net/reference/programming/features/beatdetection/
--

"ported" to XBMC by d4rk
d4rk@xbmc.org

*/

#pragma once

#include <kodi/addon-instance/Visualization.h>
#include <mutex>

#include <libprojectM/projectM.hpp>

class ATTRIBUTE_HIDDEN CVisualizationProjectM
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceVisualization
{
public:
  CVisualizationProjectM();
  ~CVisualizationProjectM() override;

  bool Start(int channels, int samplesPerSec, int bitsPerSample, std::string songName) override;
  void Render() override;
  void AudioData(const float* audioData, int audioDataLength, float *freqData, int freqDataLength) override;
  bool GetPresets(std::vector<std::string>& presets) override;
  bool LoadPreset(int select) override;
  bool PrevPreset() override;
  bool NextPreset() override;
  bool LockPreset(bool lockUnlock) override;
  int GetActivePreset() override;
  bool RandomPreset() override;
  bool IsLocked() override;
  ADDON_STATUS SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue) override;

private:
  bool InitProjectM();
  void ChoosePresetPack(int pvalue);
  void ChooseUserPresetFolder(std::string pvalue);

  projectM* m_projectM;
  projectM::Settings m_configPM;
  std::mutex m_pmMutex;
  bool m_UserPackFolder;
  std::string m_lastPresetDir;
  int m_lastPresetIdx;
#ifdef DEBUG
  unsigned int m_lastLoggedPresetIdx;
#endif
  bool m_lastLockStatus;
  bool m_shutdown = false;

#ifdef _WIN32
  bool m_presetsSet = false;
#endif

  // some projectm globals
  const static int maxSamples=512;
  const static int texsize=512;
  const static int gx=40,gy=30;
  const static int fps=100;
};

