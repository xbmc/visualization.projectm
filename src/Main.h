/*
 *  Copyright (C) 2007-2026 Team Kodi (https://kodi.tv)
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

#include <atomic>
#include <kodi/addon-instance/Visualization.h>
#include <mutex>
#include <projectM-4/playlist.h>
#include <projectM-4/projectM.h>

class ATTR_DLL_LOCAL CVisualizationProjectM : public kodi::addon::CAddonBase,
                                              public kodi::addon::CInstanceVisualization
{
public:
  CVisualizationProjectM();
  ~CVisualizationProjectM() override;

  bool Init() override;
  void Render() override;
  void AudioData(const float* audioData, size_t audioDataLength) override;
  bool GetPresets(std::vector<std::string>& presets) override;
  bool LoadPreset(int select) override;
  bool PrevPreset() override;
  bool NextPreset() override;
  bool LockPreset(bool lockUnlock) override;
  int GetActivePreset() override;
  bool RandomPreset() override;
  bool IsLocked() override;
  ADDON_STATUS SetSetting(const std::string& settingName,
                          const kodi::addon::CSettingValue& settingValue) override;

private:
  bool InitProjectM();
  void ChoosePresetPack(int pvalue);
  void ChooseUserPresetFolder(std::string pvalue);
  std::string GetBasename(std::string fullPath);
  void ReloadPlaylist();
  static void PresetSwitchedEvent(bool isHardCut, unsigned int index, void* context);
  static void ProjectMLogCallback(const char* message,
                                  projectm_log_level log_level,
                                  void* user_data);

  bool m_settingChanged{true};
  bool m_UserPackFolder{false};
  std::string m_texturePath;

  // Stored values where we get from settings.xml
  // The name and order is identical to settings.xml.
  // NOTE: Value last_preset_folder can be it a bit confusing, as it is in process the currently used preset folder.
  struct
  {
    int preset_pack{-1};
    std::string user_preset_folder;
    std::string last_preset_folder;
    std::atomic_int last_preset_idx{};
    bool last_locked_status{false};
    bool shuffle{false};
    double smooth_duration{0};
    double preset_duration{0};
    float beat_sens{0};
  } m_settings;

  projectm_handle m_projectM{nullptr};
  projectm_playlist_handle m_playlist{nullptr};
  std::recursive_mutex m_pmMutex;
  std::atomic_bool m_shutdown{false};

  // some projectm globals
  const static int gx{40};
  const static int gy{30};
  const static int fps{60};
};
