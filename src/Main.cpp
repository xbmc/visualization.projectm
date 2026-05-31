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
  unsigned int lastindex = 0;
  m_projectM->selectedPresetIndex(lastindex);
  m_shutdown = true;
  kodi::addon::SetSettingInt("last_preset_idx", lastindex);
  kodi::addon::SetSettingString("last_preset_folder", m_projectM->settings().presetURL);
  kodi::addon::SetSettingBoolean("last_locked_status", m_projectM->isPresetLocked());

  if (m_projectM)
  {
    delete m_projectM;
    m_projectM = nullptr;
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
  m_settings.quality = kodi::addon::GetSettingInt("quality");
  m_settings.smooth_duration = static_cast<double>(kodi::addon::GetSettingFloat("smooth_duration"));
  m_settings.preset_duration = static_cast<double>(kodi::addon::GetSettingFloat("preset_duration"));
  m_settings.beat_sens = kodi::addon::GetSettingFloat("beat_sens");

  m_configPM.meshX = gx;
  m_configPM.meshY = gy;
  m_configPM.fps = fps;
  m_configPM.windowWidth = Width();
  m_configPM.windowHeight = Height();
  m_configPM.aspectCorrection = true;
  m_configPM.easterEgg = 0.0;
  m_configPM.titleFontURL = kodi::addon::GetAddonPath("resources/projectM/fonts/Vera.ttf");
  m_configPM.menuFontURL = kodi::addon::GetAddonPath("resources/projectM/fonts/VeraMono.ttf");
  m_configPM.datadir = kodi::addon::GetAddonPath("resources/projectM");
  m_configPM.textureSize = m_settings.quality;
  m_configPM.shuffleEnabled = m_settings.shuffle;
  m_configPM.smoothPresetDuration = static_cast<int>(m_settings.smooth_duration);
  m_configPM.presetDuration = static_cast<int>(m_settings.preset_duration);
  m_configPM.beatSensitivity = m_settings.beat_sens;

  ChoosePresetPack(m_settings.preset_pack);
  ChooseUserPresetFolder(m_settings.user_preset_folder);

  if (!InitProjectM())
  {
    kodi::Log(ADDON_LOG_FATAL, "Failed to initialize projectM - addon will not function");
    // Object is in invalid state - all subsequent method calls must check m_projectM/m_playlist
    return false;
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
    m_projectM->pcm()->addPCMfloat_2ch(pAudioData, iAudioDataLength);
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
void CVisualizationProjectM::Render()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_projectM)
    m_projectM->renderFrame();
}

bool CVisualizationProjectM::LoadPreset(int select)
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  m_projectM->selectPreset(select);
  return true;
}

bool CVisualizationProjectM::PrevPreset()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  //  switchPreset(ALPHA_PREVIOUS, SOFT_CUT);
  if (!m_projectM->isShuffleEnabled())
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_p,
                            PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
  else
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_r,
                            PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS

  return true;
}

bool CVisualizationProjectM::NextPreset()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  //  switchPreset(ALPHA_NEXT, SOFT_CUT);
  if (!m_projectM->isShuffleEnabled())
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_n,
                            PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
  else
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_r,
                            PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
  return true;
}

bool CVisualizationProjectM::RandomPreset()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  m_projectM->setShuffleEnabled(m_configPM.shuffleEnabled);
  return true;
}

bool CVisualizationProjectM::LockPreset(bool lockUnlock)
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  m_projectM->setPresetLock(lockUnlock);
  unsigned preset;
  m_projectM->selectedPresetIndex(preset);
  m_projectM->selectPreset(preset);
  return true;
}

//-- GetPresets ---------------------------------------------------------------
// Return a list of presets to Kodi for display
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::GetPresets(std::vector<std::string>& presets)
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  int numPresets = m_projectM ? m_projectM->getPlaylistSize() : 0;
  if (numPresets > 0)
  {
    for (unsigned i = 0; i < numPresets; i++)
      presets.push_back(m_projectM->getPresetName(i));
  }
  return (numPresets > 0) ? true : false;
}

//-- GetPreset ----------------------------------------------------------------
// Return the index of the current playing preset
//-----------------------------------------------------------------------------
int CVisualizationProjectM::GetActivePreset()
{
  unsigned preset;
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_projectM && m_projectM->selectedPresetIndex(preset))
    return preset;

  return 0;
}

//-- IsLocked -----------------------------------------------------------------
// Returns true if this add-on use settings
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::IsLocked()
{
  std::unique_lock<std::recursive_mutex> lock(m_pmMutex);
  if (m_projectM)
    return m_projectM->isPresetLocked();
  else
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

          m_settings.preset_pack = newValue;
          ChoosePresetPack(newValue);
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
      else if (settingName == "last_preset_idx")
      {
        const int newValue = settingValue.GetInt();
        if (m_settings.last_preset_idx != newValue)
        {
          m_settingChanged = true;

          m_settings.last_preset_idx = newValue;
        }
      }
      else if (settingName == "last_locked_status")
      {
        const bool newValue = settingValue.GetBoolean();
        if (m_settings.last_locked_status != newValue)
        {
          m_settingChanged = true;

          m_settings.last_locked_status = newValue;
        }
      }
      else if (settingName == "shuffle")
      {
        const bool newValue = settingValue.GetBoolean();
        if (m_settings.shuffle != newValue)
        {
          m_settingChanged = true;

          m_settings.shuffle = newValue;
          m_configPM.shuffleEnabled = m_settings.shuffle;
        }
      }
      else if (settingName == "quality")
      {
        const int newValue = settingValue.GetInt();
        if (m_settings.last_preset_idx != newValue)
        {
          m_settingChanged = true;

          m_settings.quality = newValue;
          m_configPM.textureSize = m_settings.quality;
        }
      }
      else if (settingName == "smooth_duration")
      {
        const double newValue = static_cast<double>(settingValue.GetFloat());
        if (m_settings.smooth_duration != newValue)
        {
          m_settingChanged = true;

          m_settings.smooth_duration = newValue;
          m_configPM.smoothPresetDuration = static_cast<int>(m_settings.smooth_duration);
        }
      }
      else if (settingName == "preset_duration")
      {
        const double newValue = static_cast<double>(settingValue.GetFloat());
        if (m_settings.preset_duration != newValue)
        {
          m_settingChanged = true;

          m_settings.preset_duration = newValue;
          m_configPM.presetDuration = static_cast<int>(m_settings.preset_duration);
        }
      }
      else if (settingName == "beat_sens")
      {
        const float newValue = settingValue.GetFloat();
        if (m_settings.beat_sens != newValue)
        {
          m_settingChanged = true;

          m_settings.beat_sens = newValue;
          m_configPM.beatSensitivity = m_settings.beat_sens;
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
  delete m_projectM; //We are re-initializing the engine
  try
  {
    m_projectM = new projectM(m_configPM);
    if (m_configPM.presetURL ==
        m_settings
            .last_preset_folder) //If it is not the first run AND if this is the same preset pack as last time
    {
      m_projectM->setPresetLock(m_settings.last_locked_status);
      m_projectM->selectPreset(m_settings.last_preset_idx);
    }
    else
    {
      //If it is the first run or a newly chosen preset pack we choose a random preset as first
      if (m_projectM->getPlaylistSize())
        m_projectM->selectPreset((rand() % (m_projectM->getPlaylistSize())));
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
  if (pvalue == -1)
  {
    m_UserPackFolder = true;
    m_settings.preset_pack = -1;
    return;
  }

  const auto entry = installed_presets.find(pvalue);
  if (entry == installed_presets.end())
  {
    kodi::Log(ADDON_LOG_FATAL,
              "CVisualizationProjectM::%s: Should never called with unknown preset pack (%i)",
              __func__, pvalue);
    return;
  }

  m_UserPackFolder = false;
  m_settings.preset_pack = pvalue;
  m_settings.last_preset_folder = kodi::addon::GetAddonPath(entry->second.path);
  m_configPM.presetURL = m_settings.last_preset_folder;
}

void CVisualizationProjectM::ChooseUserPresetFolder(std::string pvalue)
{
  if (m_UserPackFolder && !pvalue.empty())
  {
    if (pvalue.back() == '/')
      pvalue.erase(pvalue.length() - 1, 1); //Remove "/" from the end
    m_settings.last_preset_folder = pvalue;
    m_configPM.presetURL = m_settings.last_preset_folder;
  }
}

ADDONCREATOR(CVisualizationProjectM)
