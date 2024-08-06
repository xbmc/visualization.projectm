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

#include "Main.h"

//-- Create -------------------------------------------------------------------
// Called once when the visualisation is created by Kodi. Do any setup here.
//-----------------------------------------------------------------------------
CVisualizationProjectM::CVisualizationProjectM()
{
  if (!InitProjectM())
  {
    return;
  }

  projectm_set_mesh_size(m_projectM, gx, gy);
  projectm_set_fps(m_projectM, fps);
  projectm_set_window_size(m_projectM, Width(), Height());
  projectm_set_aspect_correction(m_projectM, true);
  projectm_set_easter_egg(m_projectM, 0.0);

  auto texturePath = kodi::addon::GetAddonPath("resources/projectM/textures");
  std::vector<const char*> texturePaths = {texturePath.data()};
  projectm_set_texture_search_paths(m_projectM, texturePaths.data(), texturePaths.size());

  m_lastPresetIdx = kodi::addon::GetSettingInt("last_preset_idx");
#ifdef DEBUG
  m_lastLoggedPresetIdx = m_lastPresetIdx;
#endif

  projectm_playlist_set_shuffle(m_playlist, kodi::addon::GetSettingBoolean("shuffle"));

  m_lastLockStatus = kodi::addon::GetSettingBoolean("last_locked_status");
  m_presetDir = kodi::addon::GetSettingString("last_preset_folder");

  projectm_set_soft_cut_duration(m_projectM, static_cast<double>(kodi::addon::GetSettingFloat("smooth_duration")));
  projectm_set_preset_duration(m_projectM, static_cast<double>(kodi::addon::GetSettingFloat("preset_duration")));
  projectm_set_beat_sensitivity(m_projectM, kodi::addon::GetSettingFloat("beat_sens"));

  ChoosePresetPack(kodi::addon::GetSettingInt("preset_pack"));
  ChooseUserPresetFolder(kodi::addon::GetSettingString("user_preset_folder"));

  // Populate playlist and set initial index
  projectm_playlist_add_path(m_playlist, m_presetDir.c_str(), true, false);

  // If it is not the first run AND if this is the same preset pack as last time
  if (kodi::addon::GetSettingString("last_preset_folder", "") == m_presetDir && m_lastPresetIdx > 0)
  {
    projectm_playlist_set_position(m_playlist, m_lastPresetIdx, true);
    projectm_set_preset_locked(m_projectM, m_lastLockStatus);
  }
  else
  {
    // If it is the first run or a newly chosen preset pack we choose a random preset as first
    if (projectm_playlist_size(m_playlist) > 0)
    {
      auto shuffleEnabled = projectm_playlist_get_shuffle(m_playlist);
      projectm_playlist_set_shuffle(m_playlist, true);
      projectm_playlist_play_next(m_playlist, false);
      projectm_playlist_set_shuffle(m_playlist, shuffleEnabled);
    }
  }
}

CVisualizationProjectM::~CVisualizationProjectM()
{
  m_shutdown = true;

  if (m_playlist && m_projectM)
  {
    auto lastindex = projectm_playlist_get_position(m_playlist);
    kodi::addon::SetSettingInt("last_preset_idx", lastindex);
    kodi::addon::SetSettingString("last_preset_folder", m_presetDir);
    kodi::addon::SetSettingBoolean("last_locked_status", projectm_get_preset_locked(m_projectM));
  }

  if(m_playlist)
  {
    projectm_playlist_destroy(m_playlist);
    m_playlist = nullptr;
  }

  if (m_projectM)
  {
    projectm_destroy(m_projectM);
    m_projectM = nullptr;
  }
}

bool CVisualizationProjectM::Start(int channels, int samplesPerSec, int bitsPerSample, const std::string& songName)
{
  // Todo: Store channels, pass this value in AudioData() to projectM
#ifdef _WIN32
  InitProjectM();

  if (!m_presetsSet)
  {
    std::vector<std::string> presets;
    GetPresets(presets);
    CInstanceVisualization::TransferPresets(presets);
    m_presetsSet = true;
  }
#endif
  return true;
}

//-- Audiodata ----------------------------------------------------------------
// Called by Kodi to pass new audio data to the vis
//-----------------------------------------------------------------------------
void CVisualizationProjectM::AudioData(const float* pAudioData, size_t iAudioDataLength)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    projectm_pcm_add_float(m_projectM, pAudioData, iAudioDataLength / 2, PROJECTM_STEREO);
  }
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
void CVisualizationProjectM::Render()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    projectm_opengl_render_frame(m_projectM);
#ifdef DEBUG
    auto preset = projectm_playlist_get_position(m_playlist);
    if (m_lastLoggedPresetIdx != preset)
      CLog::Log(ADDON_LOG_DEBUG,"PROJECTM - Changed preset to: %s",g_presets[preset]);
    m_lastLoggedPresetIdx = preset;
#endif
  }
}

bool CVisualizationProjectM::LoadPreset(int select)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    projectm_playlist_set_position(m_playlist, select, true);
  }
  return true;
}

bool CVisualizationProjectM::PrevPreset()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    projectm_playlist_play_previous(m_playlist, false);
  }

  return true;
}

bool CVisualizationProjectM::NextPreset()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    projectm_playlist_play_next(m_playlist, false);
  }

  return true;
}

bool CVisualizationProjectM::RandomPreset()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    auto shuffleEnabled = projectm_playlist_get_shuffle(m_playlist);
    projectm_playlist_set_shuffle(m_playlist, true);
    projectm_playlist_play_next(m_playlist, false);
    projectm_playlist_set_shuffle(m_playlist, shuffleEnabled);
  }
  return true;
}

bool CVisualizationProjectM::LockPreset(bool lockUnlock)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    projectm_set_preset_locked(m_projectM, lockUnlock);
  }
  return true;
}

//-- GetPresets ---------------------------------------------------------------
// Return a list of presets to Kodi for display
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::GetPresets(std::vector<std::string>& presets)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (!m_playlist)
  {
    return false;
  }

  char** playlistItems = projectm_playlist_items(m_playlist, 0, projectm_playlist_size(m_playlist));

  char** item = playlistItems;
  while (*item)
  {
    presets.push_back(GetBasename(*item));
    item++;
  }

  projectm_playlist_free_string_array(playlistItems);

  return !presets.empty();
}

//-- GetPreset ----------------------------------------------------------------
// Return the index of the current playing preset
//-----------------------------------------------------------------------------
int CVisualizationProjectM::GetActivePreset()
{
  unsigned preset;
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_playlist && projectm_playlist_get_position(m_playlist))
  {
    return static_cast<int>(projectm_playlist_get_position(m_playlist));
  }

  return 0;
}

//-- IsLocked -----------------------------------------------------------------
// Returns true if this add-on use settings
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::IsLocked()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    return projectm_get_preset_locked(m_projectM);
  }
  else
  {
    return false;
  }
}

//-- UpdateSetting ------------------------------------------------------------
// Handle setting change request from Kodi
//-----------------------------------------------------------------------------
ADDON_STATUS
CVisualizationProjectM::SetSetting(const std::string& settingName, const kodi::addon::CSettingValue& settingValue)
{
  if (settingName.empty() || settingValue.empty())
  {
    return ADDON_STATUS_UNKNOWN;
  }

  {
    std::unique_lock<std::mutex> lock(m_pmMutex);

    if (!m_projectM || !m_playlist)
    {
      return ADDON_STATUS_UNKNOWN;
    }

    // It is now time to set the settings got from xmbc
    if (settingName == "shuffle")
    {
      projectm_playlist_set_shuffle(m_playlist, settingValue.GetBoolean());
    }
    else if (settingName == "last_preset_idx")
    {
      m_lastPresetIdx = settingValue.GetInt();
      projectm_playlist_set_position(m_playlist, m_lastPresetIdx, false);
    }
    else if (settingName == "last_locked_status")
    {
      m_lastLockStatus = settingValue.GetBoolean();
      projectm_set_preset_locked(m_projectM, m_lastLockStatus);
    }
    else if (settingName == "last_preset_folder")
    {
      m_presetDir = settingValue.GetString();
      ReloadPlaylist();
    }
    else if (settingName == "smooth_duration")
    {
      projectm_set_soft_cut_duration(m_projectM, static_cast<double>(settingValue.GetFloat()));
    }
    else if (settingName == "preset_duration")
    {
      projectm_set_preset_duration(m_projectM, static_cast<double>(settingValue.GetFloat()));
    }
    else if (settingName == "preset_pack")
    {
      ChoosePresetPack(settingValue.GetInt());
      if (kodi::addon::GetSettingString("last_preset_folder", "") != m_presetDir)
      {
        ReloadPlaylist();
      }
    }
    else if (settingName == "user_preset_folder")
    {
      ChooseUserPresetFolder(settingValue.GetString());
      if (kodi::addon::GetSettingString("last_preset_folder", "") != m_presetDir)
      {
        ReloadPlaylist();
      }
    }
    else if (settingName == "beat_sens")
    {
      projectm_set_beat_sensitivity(m_projectM, settingValue.GetFloat());
    }
  }
  if (settingName == "beat_sens" && !m_shutdown) // becomes changed in future by a additional value on function
  {
    if (!InitProjectM())
    {
      // The last setting value is already set so we (re)initalize
      return ADDON_STATUS_UNKNOWN;
    }
  }
  return ADDON_STATUS_OK;
}

bool CVisualizationProjectM::InitProjectM()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    projectm_playlist_connect(m_playlist, nullptr);
  }

  if (m_projectM)
  {
    projectm_destroy(m_projectM); // We are re-initializing the engine
    m_projectM = nullptr;
  }

  try
  {

    m_projectM = projectm_create();
    if (!m_projectM)
    {
      kodi::Log(ADDON_LOG_FATAL, "Could not create projectM instance.");
      return false;
    }

    if (!m_playlist)
    {
      m_playlist = projectm_playlist_create(m_projectM);
      if (!m_playlist)
      {
        projectm_destroy(m_projectM);
        m_projectM = nullptr;
        kodi::Log(ADDON_LOG_FATAL, "Could not create projectM playlist instance.");
        return false;
      }

      // Automatically update last preset index if it changes
      projectm_playlist_set_preset_switched_event_callback(m_playlist, &CVisualizationProjectM::PresetSwitchedEvent, static_cast<void*>(this));
    }
    else
    {
      // Reconnect new instance with existing playlist manager
      projectm_playlist_connect(m_playlist, m_projectM);
    }

    return true;
  }
  catch (...)
  {
    kodi::Log(ADDON_LOG_FATAL, "exception in projectM ctor");
    return false;
  }
}

void CVisualizationProjectM::ChoosePresetPack(int pvalue)
{
  switch (pvalue)
  {
    case -1:
      m_UserPackFolder = true;
      break;

    case 0:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_bltc201");
      break;

    case 1:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_milkdrop");
      break;

    case 2:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_milkdrop_104");
      break;

    case 3:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_milkdrop_200");
      break;

    case 4:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_mischa_collection");
      break;

    case 5:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_projectM");

    case 6:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_stock");
      break;

    case 7:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_tryptonaut");
      break;

    case 8:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_yin");
      break;

    case 9:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/tests");
      break;

    case 10:
      m_UserPackFolder = false;
      m_presetDir = kodi::addon::GetAddonPath("resources/projectM/presets/presets_eyetune");
      break;

    default:
      kodi::Log(ADDON_LOG_FATAL, "CVisualizationProjectM::%s: Should never called with unknown preset pack (%i)",
                __func__, pvalue);
      break;
  }
}

void CVisualizationProjectM::ChooseUserPresetFolder(std::string pvalue)
{
  if (m_UserPackFolder && !pvalue.empty())
  {
    if (pvalue.back() == '/')
    {
      pvalue.erase(pvalue.length() - 1, 1);
    }  //Remove "/" from the end
    m_presetDir = pvalue;
  }
}

std::string CVisualizationProjectM::GetBasename(std::string fullPath)
{
  auto lastSlash = fullPath.find_last_of("/\\");
  if (lastSlash != std::string::npos)
  {
    fullPath = fullPath.substr(lastSlash + 1);
  }

  auto lastExt = fullPath.find_last_of(".");
  if (lastExt != std::string::npos && lastExt != 0)
  {
    fullPath = fullPath.substr(0, lastExt);
  }

  return fullPath;
}

void CVisualizationProjectM::ReloadPlaylist()
{
  // Load new playlist and select a random preset
  projectm_playlist_clear(m_playlist);
  projectm_playlist_add_path(m_playlist, m_presetDir.c_str(), true, false);
  if (projectm_playlist_size(m_playlist) > 0)
  {
    auto shuffleEnabled = projectm_playlist_get_shuffle(m_playlist);
    projectm_playlist_set_shuffle(m_playlist, true);
    projectm_playlist_play_next(m_playlist, true);
    projectm_playlist_set_shuffle(m_playlist, shuffleEnabled);
  }
}

void CVisualizationProjectM::PresetSwitchedEvent(bool isHardCut, unsigned int index, void* context)
{
  auto that = reinterpret_cast<CVisualizationProjectM*>(context);
  that->m_lastPresetIdx = static_cast<int>(projectm_playlist_get_position(that->m_playlist));
}

ADDONCREATOR(CVisualizationProjectM)
