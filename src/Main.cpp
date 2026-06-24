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

#include "Main.h"

#include <unordered_map>

namespace
{

struct preset_info
{
  std::string path;
  uint32_t labelId;
};

constexpr int DEFAULT_PRESET = 5;

// The preset packs that are installed with the add-on.
// The key is the value of the "preset_pack" setting in settings.xml
const std::unordered_map<int, preset_info> installed_presets = {
    {0, {"resources/projectM/presets/presets_bltc201", 30020}},
    {1, {"resources/projectM/presets/presets_milkdrop", 30021}},
    {2, {"resources/projectM/presets/presets_milkdrop_104", 30022}},
    {3, {"resources/projectM/presets/presets_milkdrop_200", 30023}},
    {4, {"resources/projectM/presets/presets_mischa_collection", 30024}},
    {5, {"resources/projectM/presets/presets_projectM", 30025}},
    {6, {"resources/projectM/presets/presets_stock", 30026}},
    {7, {"resources/projectM/presets/presets_tryptonaut", 30027}},
    {8, {"resources/projectM/presets/presets_yin", 30028}},
    {9, {"resources/projectM/presets/tests", 30029}},
    {10, {"resources/projectM/presets/presets_eyetune", 30030}}};

} // namespace

CVisualizationProjectM::~CVisualizationProjectM()
{
  m_shutdown = true;

  {
    std::unique_lock<std::recursive_mutex> lock(m_pmMutex);

    if (m_playlist && m_projectM)
    {
      // Remove our used callback about preset change event from ProjectM
      projectm_playlist_set_preset_switched_event_callback(m_playlist, nullptr, nullptr);

      // Store the last used preset to have on next use.
      auto lastindex = projectm_playlist_get_position(m_playlist);
      kodi::addon::SetSettingInt("last_preset_idx", lastindex);
      kodi::addon::SetSettingString("last_preset_folder", m_settings.last_preset_folder);
      kodi::addon::SetSettingBoolean("last_locked_status", projectm_get_preset_locked(m_projectM));

      // WARNING: Within the current projectM Version 4 we have a problem that if some processing parts about becomes changed,
      // for example the preset, the screen becomes black after stop.
      // Hopefully we find a way to fix this in future, but for now we have to avoid calls to ProjectM.
      //
      // NOTE: The problem comes after unload of the add-on library, not with destroy calls below.
      //
      // !!! Use this to reproduce the fault of black screen after stop !!!
      //projectm_load_preset_file(
      //    m_projectM, "idle://Geiss & Sperl - Feedback (projectM idle HDR mix).milk", false);
    }

    if (m_playlist)
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
}

//-- Init -------------------------------------------------------------------
// Called once when the visualisation is created by Kodi. Do any setup here.
//-----------------------------------------------------------------------------

bool CVisualizationProjectM::Init()
{
  // Load all available settings from add-on.
  m_settings.preset_pack = kodi::addon::GetSettingInt("preset_pack");
  m_settings.user_preset_folder = kodi::addon::GetSettingString("user_preset_folder");
  m_settings.last_preset_folder = kodi::addon::GetSettingString("last_preset_folder");
  m_settings.last_preset_idx = kodi::addon::GetSettingInt("last_preset_idx");
  m_settings.last_locked_status = kodi::addon::GetSettingBoolean("last_locked_status");
  m_settings.shuffle = kodi::addon::GetSettingBoolean("shuffle");
  m_settings.smooth_duration = static_cast<double>(kodi::addon::GetSettingFloat("smooth_duration"));
  m_settings.preset_duration = static_cast<double>(kodi::addon::GetSettingFloat("preset_duration"));
  m_settings.beat_sens = kodi::addon::GetSettingFloat("beat_sens");

  if (!InitProjectM())
  {
    kodi::Log(ADDON_LOG_FATAL, "Failed to initialize projectM - addon will not function");
    // Object is in invalid state - all subsequent method calls must check m_projectM/m_playlist
    return false;
  }

  projectm_set_mesh_size(m_projectM, gx, gy);
  projectm_set_fps(m_projectM, fps);
  projectm_set_window_size(m_projectM, Width(), Height());
  projectm_set_aspect_correction(m_projectM, true);
  projectm_set_easter_egg(m_projectM, 0.0);

  m_texturePath = kodi::addon::GetAddonPath("resources/projectM/textures");
  std::vector<const char*> texturePaths = {m_texturePath.data()};
  projectm_set_texture_search_paths(m_projectM, texturePaths.data(), texturePaths.size());

  projectm_playlist_set_shuffle(m_playlist, m_settings.shuffle);
  projectm_set_soft_cut_duration(m_projectM, m_settings.smooth_duration);
  projectm_set_preset_duration(m_projectM, m_settings.preset_duration);
  projectm_set_beat_sensitivity(m_projectM, m_settings.beat_sens);

  // Store here the old to have after the following function calls available, as
  // it can be changed there.
  const std::string presetFolderBefore = m_settings.last_preset_folder;

  ChoosePresetPack(m_settings.preset_pack);
  ChooseUserPresetFolder(m_settings.user_preset_folder);

  // Populate playlist and set initial index
  uint32_t presetsAdded =
      projectm_playlist_add_path(m_playlist, m_settings.last_preset_folder.c_str(), true, false);
  if (presetsAdded == 0)
  {
    kodi::Log(ADDON_LOG_WARNING, "Init: Failed to load presets from: %s, falling back to default",
              m_settings.last_preset_folder.c_str());

    m_settings.last_preset_folder.clear();
    m_settings.last_preset_idx = 0;
    m_settings.preset_pack = DEFAULT_PRESET;
    ChoosePresetPack(m_settings.preset_pack);

    presetsAdded =
        projectm_playlist_add_path(m_playlist, m_settings.last_preset_folder.c_str(), true, false);
    if (presetsAdded == 0)
    {
      kodi::Log(ADDON_LOG_FATAL,
                "Init: Failed to load default presets (where should always present) from: %s - "
                "addon will not function",
                m_settings.last_preset_folder.c_str());

      return false;
    }
  }

  // If it is not the first run AND if this is the same preset pack as last time
  if (presetFolderBefore == m_settings.last_preset_folder && m_settings.last_preset_idx > 0)
  {
    const auto playlistSize = projectm_playlist_size(m_playlist);
    if (m_settings.last_preset_idx >= playlistSize)
    {
      kodi::Log(ADDON_LOG_ERROR,
                "Init: Last selected preset index %i out of range of available presets %i, falling "
                "back to first",
                static_cast<int>(m_settings.last_preset_idx), playlistSize);
      m_settings.last_preset_idx = 0;
    }

    projectm_playlist_set_position(m_playlist, m_settings.last_preset_idx, true);
    projectm_set_preset_locked(m_projectM, m_settings.last_locked_status);
  }
  else
  {
    // If it is the first run or a newly chosen preset pack we choose a random preset as first
    if (projectm_playlist_size(m_playlist) > 0)
    {
      auto shuffleEnabled = projectm_playlist_get_shuffle(m_playlist);
      projectm_playlist_set_shuffle(m_playlist, true);
      projectm_playlist_play_next(m_playlist, true);
      projectm_playlist_set_shuffle(m_playlist, shuffleEnabled);
    }
  }

  return true;
}

//-- Audiodata ----------------------------------------------------------------
// Called by Kodi to pass new audio data to the vis
//-----------------------------------------------------------------------------
void CVisualizationProjectM::AudioData(const float* pAudioData, size_t iAudioDataLength)
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    projectm_pcm_add_float(m_projectM, pAudioData, iAudioDataLength / 2,
                           static_cast<projectm_channels>(2));
  }
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
void CVisualizationProjectM::Render()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    projectm_opengl_render_frame(m_projectM);
  }
}

bool CVisualizationProjectM::LoadPreset(int select)
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    projectm_playlist_set_position(m_playlist, select, true);
  }
  return true;
}

bool CVisualizationProjectM::PrevPreset()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    projectm_playlist_play_previous(m_playlist, false);
  }

  return true;
}

bool CVisualizationProjectM::NextPreset()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_playlist)
  {
    projectm_playlist_play_next(m_playlist, false);
  }

  return true;
}

bool CVisualizationProjectM::RandomPreset()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
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
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
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
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (!m_playlist)
  {
    return false;
  }

  char** playlistItems = projectm_playlist_items(m_playlist, 0, projectm_playlist_size(m_playlist));
  if (!playlistItems)
  {
    return false;
  }

  try
  {
    char** item = playlistItems;
    while (*item)
    {
      presets.push_back(GetBasename(*item));
      item++;
    }
    projectm_playlist_free_string_array(playlistItems);
  }
  catch (...)
  {
    projectm_playlist_free_string_array(playlistItems);
    throw;
  }

  return !presets.empty();
}

//-- GetPreset ----------------------------------------------------------------
// Return the index of the current playing preset
//-----------------------------------------------------------------------------
int CVisualizationProjectM::GetActivePreset()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_playlist)
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
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    return projectm_get_preset_locked(m_projectM);
  }

  return false;
}

//-- UpdateSetting ------------------------------------------------------------
// Handle setting change request from Kodi
//-----------------------------------------------------------------------------
ADDON_STATUS CVisualizationProjectM::SetSetting(const std::string& settingName,
                                                const kodi::addon::CSettingValue& settingValue)
{
  if (settingName.empty() || settingValue.empty())
  {
    return ADDON_STATUS_UNKNOWN;
  }

  // Do only settings works if it is in process. In shutdown time makes no sense
  // to reinitialize ProjectM again.
  // This function becomes called on destruct as there are becomes some setting
  // values stored in add-ons settings.xml.
  if (!m_shutdown)
  {
    {
      std::unique_lock<std::recursive_mutex> lock(m_pmMutex);

      if (!m_projectM)
      {
        return ADDON_STATUS_UNKNOWN;
      }

      // It is now time to set the settings got from xmbc
      if (settingName == "preset_pack")
      {
        const int newValue = settingValue.GetInt();
        if (m_settings.preset_pack != newValue)
        {
          m_settingChanged = true;

          ChoosePresetPack(newValue);
          if (kodi::addon::GetSettingString("last_preset_folder", "") !=
              m_settings.last_preset_folder)
          {
            ReloadPlaylist();
          }
        }
      }
      else if (settingName == "user_preset_folder")
      {
        const std::string newValue = settingValue.GetString();
        if (m_settings.user_preset_folder != newValue && m_settings.preset_pack == -1)
        {
          m_settingChanged = true;

          m_settings.user_preset_folder = newValue;
          ChooseUserPresetFolder(newValue);
          if (kodi::addon::GetSettingString("last_preset_folder", "") !=
              m_settings.last_preset_folder)
          {
            ReloadPlaylist();
          }
        }
      }
      else if (settingName == "last_preset_folder")
      {
        const std::string newValue = settingValue.GetString();
        if (m_settings.last_preset_folder != newValue)
        {
          m_settingChanged = true;

          m_settings.last_preset_folder = newValue;
        }
      }
      else if (settingName == "last_preset_folder")
      {
        const std::string newValue = settingValue.GetString();
        if (m_settings.last_preset_folder != newValue)
        {
          m_settingChanged = true;

          m_settings.last_preset_folder = newValue;
          ReloadPlaylist();
        }
      }
      else if (settingName == "last_preset_idx")
      {
        const int newValue = settingValue.GetInt();
        if (m_settings.last_preset_idx != newValue)
        {
          m_settingChanged = true;

          m_settings.last_preset_idx = newValue;
          projectm_playlist_set_position(m_playlist, m_settings.last_preset_idx, false);
        }
      }
      else if (settingName == "last_locked_status")
      {
        const bool newValue = settingValue.GetBoolean();
        if (m_settings.last_locked_status != newValue)
        {
          m_settingChanged = true;

          m_settings.last_locked_status = newValue;
          projectm_set_preset_locked(m_projectM, m_settings.last_locked_status);
        }
      }
      else if (settingName == "shuffle")
      {
        const bool newValue = settingValue.GetBoolean();
        if (m_settings.shuffle != newValue)
        {
          m_settingChanged = true;

          m_settings.shuffle = newValue;
          projectm_playlist_set_shuffle(m_playlist, m_settings.shuffle);
        }
      }
      else if (settingName == "smooth_duration")
      {
        const double newValue = static_cast<double>(settingValue.GetFloat());
        if (m_settings.smooth_duration != newValue)
        {
          m_settingChanged = true;

          m_settings.smooth_duration = newValue;
          projectm_set_soft_cut_duration(m_projectM, m_settings.smooth_duration);
        }
      }
      else if (settingName == "preset_duration")
      {
        const double newValue = static_cast<double>(settingValue.GetFloat());
        if (m_settings.preset_duration != newValue)
        {
          m_settingChanged = true;

          m_settings.preset_duration = newValue;
          projectm_set_preset_duration(m_projectM, m_settings.preset_duration);
        }
      }
      else if (settingName == "beat_sens")
      {
        const float newValue = settingValue.GetFloat();
        if (m_settings.beat_sens != newValue)
        {
          m_settingChanged = true;

          m_settings.beat_sens = newValue;
          projectm_set_beat_sensitivity(m_projectM, m_settings.beat_sens);
        }
      }
    }

    // becomes changed in future by a additional value on function, currently we
    // use the last given value from settings.xml
    //
    // Check further about m_settingChanged, if something was changed, makes no
    // sense to restart if nothing new.
    if (settingName == "beat_sens" && m_settingChanged)
    {
      m_settingChanged = false;

      // The last setting value is already set so we (re)initalize
      if (!InitProjectM())
      {
        kodi::Log(ADDON_LOG_FATAL,
                  "Failed to reinitialize after settings change, screen rendering no more works.");
        return ADDON_STATUS_UNKNOWN;
      }
    }
  }

  return ADDON_STATUS_OK;
}

bool CVisualizationProjectM::InitProjectM()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);

  if (m_playlist)
  {
    projectm_playlist_connect(m_playlist, nullptr);
  }

  projectm_handle oldProjectM = nullptr;

  if (m_projectM)
  {
    // We are re-initializing the engine, if fails fallback to old
    oldProjectM = m_projectM;
    m_projectM = nullptr;
  }

  try
  {
    m_projectM = projectm_create();
    if (!m_projectM)
    {
      if (!oldProjectM)
      {
        kodi::Log(ADDON_LOG_FATAL, "Could not create projectM instance.");
        return false;
      }
      else
      {
        kodi::Log(ADDON_LOG_ERROR,
                  "Could not create new projectM instance, falling back to previous created one");
        m_projectM = oldProjectM;
      }
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
      projectm_playlist_set_preset_switched_event_callback(
          m_playlist, &CVisualizationProjectM::PresetSwitchedEvent, static_cast<void*>(this));
    }
    else
    {
      // Reconnect new instance with existing playlist manager
      projectm_playlist_connect(m_playlist, m_projectM);
    }

    if (oldProjectM)
    {
      projectm_destroy(oldProjectM);
    }

    return true;
  }
  catch (...)
  {
    if (!oldProjectM)
    {
      kodi::Log(ADDON_LOG_FATAL, "exception in projectM ctor");
    }
    else
    {
      kodi::Log(ADDON_LOG_ERROR,
                "exception in projectM ctor, falling back to previous created one");
      m_projectM = oldProjectM;
    }
    return false;
  }
}

void CVisualizationProjectM::ChoosePresetPack(int pvalue)
{
  if (pvalue == -1)
  {
    m_UserPackFolder = true;
    m_settings.preset_pack = -1;
    return;
  }

  const auto entry = installed_presets.find(pvalue);
  if (entry == installed_presets.end())
  {
    kodi::Log(ADDON_LOG_FATAL, "%s: Should never called with unknown preset pack (%i)", __func__,
              pvalue);
    return;
  }

  m_UserPackFolder = false;
  m_settings.preset_pack = pvalue;
  m_settings.last_preset_folder = kodi::addon::GetAddonPath(entry->second.path);
}

void CVisualizationProjectM::ChooseUserPresetFolder(std::string pvalue)
{
  if (m_UserPackFolder && !pvalue.empty())
  {
    if (pvalue.back() == '/')
    {
      pvalue.erase(pvalue.length() - 1, 1); //Remove "/" from the end
    }

    m_settings.last_preset_folder = pvalue;
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
  uint32_t presetsAdded =
      projectm_playlist_add_path(m_playlist, m_settings.last_preset_folder.c_str(), true, false);
  if (presetsAdded == 0)
  {
    kodi::Log(ADDON_LOG_WARNING, "%s: Failed to load presets from: %s, falling back to default",
              __func__, m_settings.last_preset_folder.c_str());
    m_settings.preset_pack = DEFAULT_PRESET;
    ChoosePresetPack(m_settings.preset_pack);
    presetsAdded =
        projectm_playlist_add_path(m_playlist, m_settings.last_preset_folder.c_str(), true, false);
  }
  if (presetsAdded > 0)
  {
    auto shuffleEnabled = projectm_playlist_get_shuffle(m_playlist);
    projectm_playlist_set_shuffle(m_playlist, true);
    projectm_playlist_play_next(m_playlist, true);
    projectm_playlist_set_shuffle(m_playlist, shuffleEnabled);
  }
}

void CVisualizationProjectM::PresetSwitchedEvent(bool isHardCut, unsigned int index, void* context)
{
  if (!context)
    return;
  auto that = reinterpret_cast<CVisualizationProjectM*>(context);

  std::unique_lock<std::recursive_mutex> lock(that->m_pmMutex);

  if (that->m_shutdown || !that->m_playlist)
    return;

  that->m_settings.last_preset_idx =
      static_cast<int>(projectm_playlist_get_position(that->m_playlist));
}

ADDONCREATOR(CVisualizationProjectM)
