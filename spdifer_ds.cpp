#include <math.h>
#include "guids.h"
#include "spdifer_ds.h"
#include "parsers\spdif\spdif_header.h"
#include "decss\DeCSSInputPin.h"

///////////////////////////////////////////////////////////////////////////////
// Number of DirectShow buffers.

#define DSHOW_BUFFERS 10

// uncomment this to log timing information into DirectShow log
//#define LOG_TIMING

#define VALIDATE(cond, err_code) if (!(cond)) return err_code;

void cr2crlf(char *_buf, size_t _size)
{
  size_t cnt = 0;

  char *src;
  char *dst;

  src = _buf;
  dst = _buf + _size;
  while (*src && src < dst)
  {
    if (*src == '\n')
      cnt++;
    src++;
  }

  dst = src + cnt;
  if (dst > _buf + _size)
    dst = _buf + _size;

  while (src != dst)
  {
    *dst-- = *src--;
    if (src[1] == '\n')
      *dst-- = '\r';
  }
}


CUnknown * WINAPI 
SpdiferDS::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS::CreateInstance"));
  SpdiferDS *pobj = new SpdiferDS("Spdifer", punk, phr);
  if (!pobj) *phr = E_OUTOFMEMORY;
  return pobj;
}

SpdiferDS::SpdiferDS(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr) :
  CTransformFilter(tszName, punk, CLSID_Spdifer)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x, %s)::SpdiferDS", this, tszName));

  if (!(m_pInput = new CDeCSSInputPin(this, phr))) 
  {
    *phr = E_OUTOFMEMORY;
    return;
  }

  if (!(sink = new DShowSink(this, phr))) 
  {
    delete m_pInput; 
    m_pInput = 0; 
    *phr = E_OUTOFMEMORY;
    return;
  }
  else
    m_pOutput = sink;

  sync = false;
  time = 0;
  sync_threshold = 0.100;

  formats = FORMAT_MASK_AC3;
  spdif_as_pcm = false;
  spdif_check_sr = true;
  spdif_allow_48 = true;
  spdif_allow_44 = false;
  spdif_allow_32 = false;
  reinit = 0;

  spdifer.set_dts_mode(DTS_MODE_PADDED);

  // Read filter options
  RegistryKey reg(REG_KEY);
  load_params(&reg);

  *phr = S_OK;
}

SpdiferDS::~SpdiferDS()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::~SpdiferDS", this));
}

void 
SpdiferDS::reset()
{
  sync = false;
  time = 0;
  spdifer.reset();
}

bool        
SpdiferDS::set_input(const CMediaType &_mt)
{
  Speakers spk_tmp;
  return mt2spk(_mt, spk_tmp) && set_input(spk_tmp);
}

bool        
SpdiferDS::set_input(Speakers _in_spk)
{
  reset();

  if (!spdifer.set_input(_in_spk))
  {
    DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::set_input(%s %s %iHz): failed", this,
      _in_spk.mode_text(), _in_spk.format_text(), _in_spk.sample_rate));
    return false;
  }

  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::set_input(%s %s %iHz): succeeded", this,
    _in_spk.mode_text(), _in_spk.format_text(), _in_spk.sample_rate));

  return true;
}

bool
SpdiferDS::process_chunk(const Chunk *_chunk)
{
#ifdef LOG_TIMING
  if (_chunk->sync)
  { DbgLog((LOG_TRACE, 3, "-> timestamp: %ims", int(_chunk->time * 1000))); }
#endif

  if (!spdifer.process(_chunk))
  {
    DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::process_chunk(): spdifer.process() failed!", this));
    return false;
  }

  Chunk chunk;
  while (!spdifer.is_empty())
  {
    if (!spdifer.get_chunk(&chunk))
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::process_chunk(): spdifer.get_chunk() failed!", this));
      return false;
    }

    ///////////////////////////////////////////////////////
    // Synchronization

    if (chunk.sync)
    {
      if (!sync)
      {
        sync = true;
        time = chunk.time;
#ifdef LOG_TIMING
        DbgLog((LOG_TRACE, 3, "-- got sync"));
#endif
      }

      if (fabs(chunk.time - time) > sync_threshold)
      {
        time = chunk.time;
#ifdef LOG_TIMING
        DbgLog((LOG_TRACE, 3, "-- resync"));
#endif
      }

    }

    if (sync)
    {
#ifdef LOG_TIMING
      if (chunk.sync)
      { DbgLog((LOG_TRACE, 3, "<- timestamp: %ims\tdelta: %ims", int(time * 1000), int((chunk.time - time) * 1000))); }
      else
      { DbgLog((LOG_TRACE, 3, "<- timestamp: %ims", int(time * 1000))); }
#endif
      chunk.set_sync(sync, time);
      if (chunk.size > 0)
        time += vtime_t(chunk.size / 4) / chunk.spk.sample_rate;
    }

    ///////////////////////////////////////////////////////
    // SPDIF as PCM

    if (spdif_as_pcm && (chunk.spk.format == FORMAT_SPDIF))
      chunk.spk = Speakers(FORMAT_PCM16, MODE_STEREO, chunk.spk.sample_rate);

    ///////////////////////////////////////////////////////
    // Send the chunk

    if (!sink->process(&chunk))
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::process_chunk(): sink->process() failed!", this));
      return false;
    }
  }
  return true;
}

bool
SpdiferDS::flush()
{
  Chunk chunk;
  chunk.set_empty(spdifer.get_input(), false, 0, true);
  return process_chunk(&chunk);
}



STDMETHODIMP 
SpdiferDS::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
  CheckPointer(ppv, E_POINTER);

  if (riid == IID_ISpdifer)
    return GetInterface((ISpdifer *) this, ppv);

  if (riid == IID_ISpecifyPropertyPages)
    return GetInterface((ISpecifyPropertyPages *) this, ppv);

  return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}


///////////////////////////////////////////////////////////////////////////////
////////////////////////////// DATA FLOW //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

HRESULT
SpdiferDS::Receive(IMediaSample *in)
{
  CAutoLock lock(&m_csReceive);

  uint8_t *buf;
  int buf_size;
  vtime_t time;

  Chunk chunk;

  /////////////////////////////////////////////////////////
  // Dynamic input format change

  CMediaType *mt;
  if (in->GetMediaType((_AMMediaType**)&mt) == S_OK)
  {
    if (*mt->FormatType() != FORMAT_WaveFormatEx)
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::Receive(): Input format change to non-audio format", this));
      return E_FAIL;
    }

    Speakers in_spk;
    if (!mt2spk(*mt, in_spk))
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::Receive(): Input format change to unsupported format", this));
      return E_FAIL;
    }

    if (spdifer.get_input() != in_spk)
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::Receive(): Input format change", this));
      flush();
      if (!set_input(in_spk))
        return VFW_E_INVALIDMEDIATYPE;
    }
  }

  Speakers in_spk = spdifer.get_input();

  /////////////////////////////////////////////////////////
  // Discontinuity

  if (in->IsDiscontinuity() == S_OK)
  {
    DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::Receive(): Discontinuity", this));
    // Send buffered samples downstream and mark next sample as discontinuity
    flush();
    reset();
    sink->send_discontinuity();
  }

  /////////////////////////////////////////////////////////
  // Data

  in->GetPointer((BYTE**)&buf);
  buf_size = in->GetActualDataLength();

  /////////////////////////////////////////////////////////
  // Fill chunk

  chunk.set_rawdata(in_spk, buf, buf_size);

  /////////////////////////////////////////////////////////
  // Timing

  REFERENCE_TIME begin, end;
  switch (in->GetTime(&begin, &end))
  {
    case S_OK:
    case VFW_S_NO_STOP_TIME:
      time = vtime_t(begin) / 10000000;
      chunk.set_sync(true, time);
      break;
  }

  /////////////////////////////////////////////////////////
  // Process

  sink->reset_hresult();
  process_chunk(&chunk);

  if FAILED(sink->get_hresult())
    return sink->get_hresult();
  else
    return S_OK;
}

HRESULT 
SpdiferDS::StartStreaming()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::StartStreaming()", this));

  // Reset before starting a new stream
  CAutoLock lock(&m_csReceive);
  reset();

  return CTransformFilter::StartStreaming();
}

HRESULT 
SpdiferDS::StopStreaming()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::StopStreaming()", this));
  return CTransformFilter::StopStreaming();
}

HRESULT 
SpdiferDS::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::NewSegment(%ims, %ims)", this, int(tStart/10000), int(tStop/10000)));

  // We have to reset because we may need to 
  // drop incomplete frame in the decoder

  CAutoLock lock(&m_csReceive);
  reset();

  return CTransformFilter::NewSegment(tStart, tStop, dRate);
}

HRESULT 
SpdiferDS::EndOfStream()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::EndOfStream()", this));

  // Syncronize with streaming thread 
  // (wait for all data to process)

  CAutoLock lock(&m_csReceive);

  // Force flushing of internal buffers of 
  // processing chain.

  flush();
  reset();

  // Send end-of-stream downstream to indicate that we have no
  // more samples to send.
  return CTransformFilter::EndOfStream();
}

HRESULT 
SpdiferDS::BeginFlush()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::BeginFlush()", this));

  // Serialize with state changes
  CAutoLock filter_lock(&m_csFilter);

  // Send BeginFlush() downstream to release all holding samples
  HRESULT hr = CTransformFilter::BeginFlush();
  if FAILED(hr) return hr;

  // Now we can be sure that Receive() at streaming thread is 
  // unblocked waiting for GetBuffer so we can now serialize 
  // with streaming thread

  CAutoLock streaming_lock(&m_csReceive);

  // Now we can be sure that Receive() (or other call at streaming thread) 
  // is finished and all data is processed.

  reset();

  // All internal processing buffers are now dropped. So we can 
  // now correctly start processing from new position.

  return S_OK;
}
HRESULT 
SpdiferDS::EndFlush()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::EndFlush()", this));

  // Syncronize with streaming thread 
  // (wait for all data to process)
  CAutoLock lock(&m_csReceive);

  return CTransformFilter::EndFlush();
}

STDMETHODIMP 
SpdiferDS::Stop()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::Stop()", this));
  return CTransformFilter::Stop();
}
STDMETHODIMP 
SpdiferDS::Pause()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::Pause()", this));
  return CTransformFilter::Pause();
}
STDMETHODIMP 
SpdiferDS::Run(REFERENCE_TIME tStart)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::Run(%ims)", this, int(tStart/10000)));
  HRESULT hr = CTransformFilter::Run(tStart);
  if FAILED(hr)
    return hr;

  if (reinit)
  {
    // Quick hack to overcome 2 'play/pause' problems:
    //
    // Some sound cards mess channel mapping after pause: channels are
    // shifted around: left to center, center ro right, etc.
    // 
    // Some sound cards (I have found it on AD1985) have a bug with pausing
    // of SPDIF playback: after pause or seeking SPDIF transmission disappears
    // at all. The reason is a bug in sound card driver: when Pause() is
    // called on DirectSound's SPDIF playback buffer sound card switches to
    // PCM mode and does not switch back to SPDIF when playback is resumed.
    // The only way to continue playback is to reopen SPDIF output. 
    //
    // To force the renderer to reopen audio output we send a portion of 
    // standard stereo 16bit PCM data (all sound cards can handle it good) so
    // forcing the renderer to close current audio output and open stereo PCM
    // playback. After this we may continue normal operation but should reset
    // DVDGraph's processing chain to force DVDGraph to re-check possibility
    // of SPDIF output.
    //
    // Also discontiniuity flag should be sent with next normal output sample
    // to force the renderer to sync time correctly because excessive PCM
    // output and format switching may produce desynchronization.
    //
    // This method is a 'quick hack' because it breaks normal DirectShow
    // data flow and produces glitches on seeking and pause.

    CAutoLock lock(&m_csReceive);

    uint8_t *buf = new uint8_t[reinit * 4];
    memset(buf, 0, reinit * 4);

    Chunk chunk;
    chunk.set_rawdata(Speakers(FORMAT_PCM16, MODE_STEREO, spdifer.get_input().sample_rate, 32767), buf, reinit * 4);

    BeginFlush();
    EndFlush();
    sink->process(&chunk);
    BeginFlush();
    EndFlush();
  }

  return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
////////////////////////////// PIN CONNECTIION ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


HRESULT 
SpdiferDS::GetMediaType(int i, CMediaType *_mt)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::GetMediaType #%i", this, i));
  VALIDATE(m_pInput->IsConnected(), E_UNEXPECTED);
  VALIDATE(i >= 0, E_INVALIDARG);

  if (spdif_as_pcm)
  {
    // PCM output
    CMediaType mt;
    Speakers spk(FORMAT_PCM16, MODE_STEREO, spdifer.get_input().sample_rate);
    if (!i--) return spk2mt(spk, *_mt, false)? NOERROR: E_FAIL;
  }
  else
  {
    // SPDIF output
    CMediaType mt;
    Speakers spk(FORMAT_SPDIF, 0, spdifer.get_input().sample_rate);
    if (!i--) return spk2mt(spk, *_mt, false)? NOERROR: E_FAIL;
    if (!i--) return spk2mt(spk, *_mt, true)? NOERROR: E_FAIL;
  }

  return VFW_S_NO_MORE_ITEMS;
}


HRESULT 
SpdiferDS::CheckInputType(const CMediaType *mt)
{
  if (m_pInput->IsConnected() == TRUE)
  {
    // If input is already connected agree with current media type
    CMediaType out_mt;
    m_pInput->ConnectionMediaType(&out_mt);
    if (*mt == out_mt)
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckInputType: No change...", this));
      return S_OK;
    }
  }

  Speakers spk_tmp;

  if (!mt2spk(*mt, spk_tmp))
  {
    DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckInputType(): cannot determine format", this));
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if ((formats & FORMAT_MASK(spk_tmp.format)) == 0)
  {
    DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckInputType(%s %s %iHz): disallowed format", this,
      spk_tmp.mode_text(), spk_tmp.format_text(), spk_tmp.sample_rate));
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if (spdif_check_sr)
    if ((!spdif_allow_48 || spk_tmp.sample_rate != 48000) && 
        (!spdif_allow_44 || spk_tmp.sample_rate != 44100) && 
        (!spdif_allow_32 || spk_tmp.sample_rate != 32000))
  {
    DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckInputType(%s %s %iHz): disallowed sample rate", this,
      spk_tmp.mode_text(), spk_tmp.format_text(), spk_tmp.sample_rate));
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckInputType(%s %s %iHz): Ok...", this,
    spk_tmp.mode_text(), spk_tmp.format_text(), spk_tmp.sample_rate));
  return S_OK;
} 

HRESULT 
SpdiferDS::CheckOutputType(const CMediaType *mt)
{
  if (m_pOutput->IsConnected() == TRUE)
  {
    // If output is already connected agree with current media type
    CMediaType out_mt;
    m_pOutput->ConnectionMediaType(&out_mt);
    if (*mt == out_mt)
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckOutputType: No change...", this));
      return S_OK;
    }
  }

  // Agree with our proposed media types
  int i = 0;
  CMediaType out_mt;
  while (GetMediaType(i++, &out_mt) == S_OK)
    if (*mt == out_mt)
    {
      DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckOutputType: Ok...", this));
      return S_OK;
    }

  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::CheckOutputType(): Not our type", this));
  return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT 
SpdiferDS::CheckTransform(const CMediaType *mt_in, const CMediaType *mt_out)
{
  DbgLog((LOG_TRACE, 3, "> SpdiferDS(%x)::CheckTransform", this));

  if FAILED(CheckInputType(mt_in))
  {
    DbgLog((LOG_TRACE, 3, "< SpdiferDS(%x)::CheckTransform(): Input type rejected", this));
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if FAILED(CheckOutputType(mt_out))
  {
    DbgLog((LOG_TRACE, 3, "< SpdiferDS(%x)::CheckTransform(): Output type rejected", this));
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  DbgLog((LOG_TRACE, 3, "< SpdiferDS(%x)::CheckTransform: Ok...", this));
  return S_OK;
}

HRESULT 
SpdiferDS::SetMediaType(PIN_DIRECTION direction, const CMediaType *mt)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::SetMediaType(%s)", this, direction == PINDIR_INPUT? "input": "output"));

  if (direction == PINDIR_INPUT)
  {
    if FAILED(CheckInputType(mt))
      return E_FAIL;

    if (!set_input(*mt))
      return E_FAIL;
  }

  if (direction == PINDIR_OUTPUT)
  {
    if FAILED(CheckOutputType(mt))
      return E_FAIL;
  }

  return S_OK;
}

HRESULT                     
SpdiferDS::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDS(%x)::DecideBufferSize", this));

  ASSERT(pAlloc);
  ASSERT(pProperties);
  HRESULT hr = NOERROR;

  pProperties->cBuffers = DSHOW_BUFFERS;
  pProperties->cbBuffer = (long)spdif_header.max_frame_size();

  ALLOCATOR_PROPERTIES Actual;
  if FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))
    return hr;

  if (pProperties->cBuffers > Actual.cBuffers ||
      pProperties->cbBuffer > Actual.cbBuffer)
    return E_FAIL;

  return NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///
/// ISpecifyPropertyPages
///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP 
SpdiferDS::GetPages(CAUUID *pPages)
{
  pPages->cElems = 2;
  pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
  if (pPages->pElems == NULL)
    return E_OUTOFMEMORY;

  (pPages->pElems)[0] = CLSID_SpdiferMain;
  (pPages->pElems)[1] = CLSID_SpdiferAbout;
  return NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///
/// ISpdifer
///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Current stream info
STDMETHODIMP SpdiferDS::get_info(char *_buf, size_t _size, int *_frames, int *_errors)
{
  if (_buf && _size)
  {
    spdifer.get_info(_buf, _size);
    cr2crlf(_buf, _size);
  }
  if (_frames) *_frames = spdifer.get_frames();
  if (_errors) *_errors = spdifer.get_errors();
  return S_OK;
}

// Input formats to accept (formats bitmask)
STDMETHODIMP SpdiferDS::get_formats(int *_formats)
{
  VALIDATE(_formats != NULL, E_INVALIDARG);
  *_formats = formats;
  return S_OK;
}

STDMETHODIMP SpdiferDS::set_formats(int _formats)
{
  formats = _formats;
  return S_OK;
}

// SPDIF/DTS output mode
STDMETHODIMP SpdiferDS::get_dts_mode(int *_dts_mode)
{
  VALIDATE(_dts_mode != NULL, E_INVALIDARG);
  *_dts_mode = spdifer.get_dts_mode();
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_dts_mode(int _dts_mode)
{
  spdifer.set_dts_mode(_dts_mode);
  return S_OK;
}

// SPDIF/DTS conversion
STDMETHODIMP SpdiferDS::get_dts_conv(int *_dts_conv)
{
  VALIDATE(_dts_conv != NULL, E_INVALIDARG);
  *_dts_conv = spdifer.get_dts_conv();
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_dts_conv(int _dts_conv)
{
  spdifer.set_dts_conv(_dts_conv);
  return S_OK;
}

// SPDIF as PCM output
STDMETHODIMP SpdiferDS::get_spdif_as_pcm(bool *_spdif_as_pcm)
{
  VALIDATE(_spdif_as_pcm != NULL, E_INVALIDARG);
  *_spdif_as_pcm = spdif_as_pcm;
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_spdif_as_pcm(bool _spdif_as_pcm)
{
  spdif_as_pcm = _spdif_as_pcm;
  return S_OK;
}

// SPDIF check sample rate
STDMETHODIMP SpdiferDS::get_spdif_check_sr(bool *_spdif_check_sr)
{
  VALIDATE(_spdif_check_sr != NULL, E_INVALIDARG);
  *_spdif_check_sr = spdif_check_sr;
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_spdif_check_sr(bool _spdif_check_sr)
{
  _spdif_check_sr = spdif_check_sr;
  return S_OK;
}
STDMETHODIMP SpdiferDS::get_spdif_allow_48(bool *_spdif_allow_48)
{
  VALIDATE(_spdif_allow_48 != NULL, E_INVALIDARG);
  *_spdif_allow_48 = spdif_allow_48;
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_spdif_allow_48(bool _spdif_allow_48)
{
  spdif_allow_48 = _spdif_allow_48;
  return S_OK;
}
STDMETHODIMP SpdiferDS::get_spdif_allow_44(bool *_spdif_allow_44)
{
  VALIDATE(_spdif_allow_44 != NULL, E_INVALIDARG);
  *_spdif_allow_44 = spdif_allow_44;
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_spdif_allow_44(bool _spdif_allow_44)
{
  spdif_allow_44 = _spdif_allow_44;
  return S_OK;
}
STDMETHODIMP SpdiferDS::get_spdif_allow_32(bool *_spdif_allow_32)
{
  VALIDATE(_spdif_allow_32 != NULL, E_INVALIDARG);
  *_spdif_allow_32 = spdif_allow_32;
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_spdif_allow_32(bool _spdif_allow_32)
{
  spdif_allow_32 = _spdif_allow_32;
  return S_OK;
}

// Reinit after seek/pause option
STDMETHODIMP SpdiferDS::get_reinit(int *_reinit)
{
  VALIDATE(_reinit != NULL, E_INVALIDARG);
  *_reinit = reinit;
  return S_OK;
}
STDMETHODIMP SpdiferDS::set_reinit(int _reinit)
{
  reinit = _reinit;
  return S_OK;
}

// Load/save settings
STDMETHODIMP SpdiferDS::set_params(SpdiferParams *_params)
{
  VALIDATE(_params != NULL, E_INVALIDARG);
  formats         = _params->formats;
  spdifer.set_dts_mode(_params->dts_mode);
  spdifer.set_dts_conv(_params->dts_conv);
  spdif_as_pcm    = _params->spdif_as_pcm;
  spdif_check_sr  = _params->spdif_check_sr;
  spdif_allow_48  = _params->spdif_allow_48;
  spdif_allow_44  = _params->spdif_allow_44;
  spdif_allow_32  = _params->spdif_allow_32;
  reinit          = _params->reinit;
  return S_OK;
}
STDMETHODIMP SpdiferDS::get_params(SpdiferParams *_params)
{
  VALIDATE(_params != NULL, E_INVALIDARG);
  _params->formats        = formats;
  _params->dts_mode       = spdifer.get_dts_mode();
  _params->dts_conv       = spdifer.get_dts_conv();
  _params->spdif_as_pcm   = spdif_as_pcm;
  _params->spdif_check_sr = spdif_check_sr;
  _params->spdif_allow_48 = spdif_allow_48;
  _params->spdif_allow_44 = spdif_allow_44;
  _params->spdif_allow_32 = spdif_allow_32;
  _params->reinit         = reinit;
  return S_OK;
}

STDMETHODIMP SpdiferDS::load_params(Config *_config)
{
  RegistryKey reg(REG_KEY);
  if (!_config) _config = &reg;

  int dts_mode = spdifer.get_dts_mode();
  int dts_conv = spdifer.get_dts_conv();

  _config->get_int32("formats", formats);
  _config->get_int32("dts_mode", dts_mode);
  _config->get_int32("dts_conv", dts_conv);
  _config->get_bool("spdif_as_pcm", spdif_as_pcm);
  _config->get_bool("spdif_check_sr", spdif_check_sr);
  _config->get_bool("spdif_allow_48", spdif_allow_48);
  _config->get_bool("spdif_allow_44", spdif_allow_44);
  _config->get_bool("spdif_allow_32", spdif_allow_32);
  _config->get_int32("reinit", reinit);

  spdifer.set_dts_mode(dts_mode);
  spdifer.set_dts_conv(dts_conv);
  return S_OK;
}

STDMETHODIMP SpdiferDS::save_params(Config *_config)
{
  RegistryKey reg;
  if (!_config) 
  {
    reg.create_key(REG_KEY);
    _config = &reg;
  }

  int dts_mode = spdifer.get_dts_mode();
  int dts_conv = spdifer.get_dts_conv();

  _config->set_int32("formats", formats);
  _config->set_int32("dts_mode", dts_mode);
  _config->set_int32("dts_conv", dts_conv);
  _config->set_bool("spdif_as_pcm", spdif_as_pcm);
  _config->set_bool("spdif_check_sr", spdif_check_sr);
  _config->set_bool("spdif_allow_48", spdif_allow_48);
  _config->set_bool("spdif_allow_44", spdif_allow_44);
  _config->set_bool("spdif_allow_32", spdif_allow_32);
  _config->set_int32("reinit", reinit);
  return S_OK;
}
