/*
 *      Copyright (C) 2007-2019 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
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
// Called once when the visualisation is created by XBMC. Do any setup here.
//-----------------------------------------------------------------------------
CVisualizationProjectM::CVisualizationProjectM()
  : m_projectM(nullptr),
    m_UserPackFolder(false)
{
  m_configPM.meshX = gx;
  m_configPM.meshY = gy;
  m_configPM.fps = fps;
  m_configPM.windowWidth = Width();
  m_configPM.windowHeight = Height();
  m_configPM.aspectCorrection = true;
  m_configPM.easterEgg = 0.0;
  m_configPM.titleFontURL = kodi::GetAddonPath("resources/projectM/fonts/Vera.ttf");
  m_configPM.menuFontURL = kodi::GetAddonPath("resources/projectM/fonts/VeraMono.ttf");
  m_configPM.datadir = kodi::GetAddonPath("resources/projectM");
  m_lastPresetIdx = kodi::GetSettingInt("last_preset_idx");
  m_lastLoggedPresetIdx = m_lastPresetIdx;

  m_configPM.textureSize = kodi::GetSettingInt("quality");
  m_configPM.shuffleEnabled = kodi::GetSettingBoolean("shuffle");

  m_lastLockStatus = kodi::GetSettingBoolean("last_locked_status");
  m_lastPresetDir = kodi::GetSettingString("last_preset_folder");
  m_configPM.smoothPresetDuration = kodi::GetSettingInt("smooth_duration");
  m_configPM.presetDuration = kodi::GetSettingInt("preset_duration");

  ChoosePresetPack(kodi::GetSettingInt("preset_pack"));
  ChooseUserPresetFolder(kodi::GetSettingString("user_preset_folder"));
  m_configPM.beatSensitivity = kodi::GetSettingInt("beat_sens") * 2;

  InitProjectM();
}

CVisualizationProjectM::~CVisualizationProjectM()
{
  unsigned int lastindex = 0;
  m_projectM->selectedPresetIndex(lastindex);
  m_shutdown = true;
  kodi::SetSettingInt("last_preset_idx", lastindex);
  kodi::SetSettingString("last_preset_folder", m_projectM->settings().presetURL);
  kodi::SetSettingBoolean("last_locked_status", m_projectM->isPresetLocked());

  if (m_projectM)
  {
    delete m_projectM;
    m_projectM = nullptr;
  }
}

//-- Audiodata ----------------------------------------------------------------
// Called by XBMC to pass new audio data to the vis
//-----------------------------------------------------------------------------
void CVisualizationProjectM::AudioData(const float* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_projectM)
    m_projectM->pcm()->addPCMfloat_2ch(pAudioData, iAudioDataLength);
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
void CVisualizationProjectM::Render()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if (m_projectM)
  {
    m_projectM->renderFrame();
      unsigned preset;
      m_projectM->selectedPresetIndex(preset);
//      if (m_lastLoggedPresetIdx != preset)
//        CLog::Log(LOGDEBUG,"PROJECTM - Changed preset to: %s",g_presets[preset]);
      m_lastLoggedPresetIdx = preset;
  }
}

bool CVisualizationProjectM::LoadPreset(int select)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  m_projectM->selectPreset(select);
  return true;
}

bool CVisualizationProjectM::PrevPreset()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
//  switchPreset(ALPHA_PREVIOUS, SOFT_CUT);
  if (!m_projectM->isShuffleEnabled())
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_p, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
  else
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_r, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS

  return true;
}

bool CVisualizationProjectM::NextPreset()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
//  switchPreset(ALPHA_NEXT, SOFT_CUT);
  if (!m_projectM->isShuffleEnabled())
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_n, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
  else
    m_projectM->key_handler(PROJECTM_KEYDOWN, PROJECTM_K_r, PROJECTM_KMOD_CAPS); //ignore PROJECTM_KMOD_CAPS
  return true;
}

bool CVisualizationProjectM::RandomPreset()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  m_projectM->setShuffleEnabled(m_configPM.shuffleEnabled);
  return true; 
}

bool CVisualizationProjectM::LockPreset(bool lockUnlock)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  m_projectM->setPresetLock(lockUnlock);
  unsigned preset;
  m_projectM->selectedPresetIndex(preset);
  m_projectM->selectPreset(preset);
  return true; 
}

//-- GetPresets ---------------------------------------------------------------
// Return a list of presets to XBMC for display
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::GetPresets(std::vector<std::string>& presets)
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
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
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if(m_projectM && m_projectM->selectedPresetIndex(preset))
    return preset;

  return 0;
}

//-- IsLocked -----------------------------------------------------------------
// Returns true if this add-on use settings
//-----------------------------------------------------------------------------
bool CVisualizationProjectM::IsLocked()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  if(m_projectM)
    return m_projectM->isPresetLocked();
  else
    return false;
}

//-- UpdateSetting ------------------------------------------------------------
// Handle setting change request from XBMC
//-----------------------------------------------------------------------------
ADDON_STATUS CVisualizationProjectM::SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue)
{
  if (settingName.empty() || settingValue.empty())
    return ADDON_STATUS_UNKNOWN;

  {
    std::unique_lock<std::mutex> lock(m_pmMutex);

    // It is now time to set the settings got from xmbc
    if (settingName == "quality")
      m_configPM.textureSize = settingValue.GetInt();
    else if (settingName == "shuffle")
      m_configPM.shuffleEnabled = settingValue.GetBoolean();
    else if (settingName == "last_preset_idx")
      m_lastPresetIdx = settingValue.GetInt();
    else if (settingName == "last_locked_status")
      m_lastLockStatus = settingValue.GetBoolean();
    else if (settingName == "last_preset_folder")
      m_lastPresetDir = settingValue.GetString();
    else if (settingName == "smooth_duration")
      m_configPM.smoothPresetDuration = (settingValue.GetInt() * 5 + 5);
    else if (settingName == "preset_duration")
      m_configPM.presetDuration = (settingValue.GetInt() * 5 + 5);
    else if (settingName == "preset_pack")
      ChoosePresetPack(settingValue.GetInt());
    else if (settingName == "user_preset_folder")
      ChooseUserPresetFolder(settingValue.GetString());
    else if (settingName == "beat_sens")
      m_configPM.beatSensitivity = settingValue.GetInt() * 2;
  }
  if (settingName == "beat_sens" && !m_shutdown) // becomes changed in future by a additional value on function
  {
    if (!InitProjectM())    //The last setting value is already set so we (re)initalize
      return ADDON_STATUS_UNKNOWN;
  }
  return ADDON_STATUS_OK;
}

bool CVisualizationProjectM::InitProjectM()
{
  std::unique_lock<std::mutex> lock(m_pmMutex);
  delete m_projectM; //We are re-initializing the engine
  try
  {
    m_projectM = new projectM(m_configPM);
    if (m_configPM.presetURL == m_lastPresetDir)  //If it is not the first run AND if this is the same preset pack as last time
    {
      m_projectM->setPresetLock(m_lastLockStatus);
      m_projectM->selectPreset(m_lastPresetIdx);
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
  switch (pvalue)
  {
    case -1:
      m_UserPackFolder = true;
      break;

    case 0:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_bltc201");
      break;

    case 1:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_milkdrop");
      break;

    case 2:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_milkdrop_104");
      break;

    case 3:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_milkdrop_200");
      break;

    case 4:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_mischa_collection");
      break;

    case 5:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_projectM");

    case 6:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_stock");
      break;

    case 7:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_tryptonaut");
      break;

    case 8:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/presets_yin");
      break;

    case 9:
      m_UserPackFolder = false;
      m_configPM.presetURL = kodi::GetAddonPath("resources/projectM/presets/tests");
      break;

    default:
      kodi::Log(ADDON_LOG_FATAL, "CVisualizationProjectM::%s: Should never called with unknown preset pack (%i)", __func__, pvalue);
      break;
  }
}

void CVisualizationProjectM::ChooseUserPresetFolder(std::string pvalue)
{
  if (m_UserPackFolder && !pvalue.empty())
  {
    if (pvalue.back() == '/')
      pvalue.erase(pvalue.length()-1,1);  //Remove "/" from the end
    m_configPM.presetURL = pvalue;
  }
}

ADDONCREATOR(CVisualizationProjectM)
