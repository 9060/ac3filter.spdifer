/*
  SpdiferDS class
*/


#ifndef SPDIFER_DS_H
#define SPDIFER_DS_H

#include <streams.h>
#include "filters\spdifer.h"
#include "sink\sink_dshow.h"

class SpdiferDS : public CTransformFilter, public ISpdifer, public ISpecifyPropertyPages
{
protected:
  Spdifer spdifer;
  DShowSink *sink;

  bool sync;
  vtime_t time;
  vtime_t sync_threshold;

  int  formats;
  bool spdif_as_pcm;
  bool spdif_check_sr;
  bool spdif_allow_48;
  bool spdif_allow_44;
  bool spdif_allow_32;
  int  reinit;

  bool set_input(const CMediaType &mt);
  bool set_input(Speakers in_spk);

  bool process_chunk(const Chunk *chunk);
  bool flush();
  void reset();

private:
  SpdiferDS(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
  ~SpdiferDS();

public:
  DECLARE_IUNKNOWN;
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

  /////////////////////////////////////////////////////////
  // CTransformFilter

  HRESULT Receive(IMediaSample *in);

  HRESULT EndOfStream();
  HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

  HRESULT StartStreaming();
  HRESULT StopStreaming();

  HRESULT BeginFlush();
  HRESULT EndFlush();

  STDMETHODIMP Stop();
  STDMETHODIMP Pause();
  STDMETHODIMP Run(REFERENCE_TIME tStart);

  HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
  HRESULT CheckInputType(const CMediaType *mt);
  HRESULT CheckOutputType(const CMediaType *mt);
  HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
  HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *mt);

  HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);


  /////////////////////////////////////////////////////////
  // ISpecifyPropertyPages

  STDMETHODIMP GetPages(CAUUID *pPages);

  /////////////////////////////////////////////////////////
  // ISpdiferDS

  // Current stream info
  STDMETHODIMP get_info(char *buf, size_t size, int *frames, int *errors);

  // Input formats to accept (formats bitmask)
  STDMETHODIMP get_formats(int *formats);
  STDMETHODIMP set_formats(int  formats);

  // SPDIF/DTS output mode
  STDMETHODIMP get_dts_mode(int *dts_mode);
  STDMETHODIMP set_dts_mode(int  dts_mode);

  // SPDIF/DTS conversion
  STDMETHODIMP get_dts_conv(int *dts_conv);
  STDMETHODIMP set_dts_conv(int  dts_conv);

  // SPDIF as PCM output
  STDMETHODIMP get_spdif_as_pcm(bool *spdif_as_pcm);
  STDMETHODIMP set_spdif_as_pcm(bool  spdif_as_pcm);

  // SPDIF check sample rate
  STDMETHODIMP get_spdif_check_sr(bool *spdif_check_sr);
  STDMETHODIMP set_spdif_check_sr(bool  spdif_check_sr);
  STDMETHODIMP get_spdif_allow_48(bool *spdif_allow_48);
  STDMETHODIMP set_spdif_allow_48(bool  spdif_allow_48);
  STDMETHODIMP get_spdif_allow_44(bool *spdif_allow_44);
  STDMETHODIMP set_spdif_allow_44(bool  spdif_allow_44);
  STDMETHODIMP get_spdif_allow_32(bool *spdif_allow_32);
  STDMETHODIMP set_spdif_allow_32(bool  spdif_allow_32);

  // Reinit after seek/pause option
  STDMETHODIMP get_reinit(int *reinit);
  STDMETHODIMP set_reinit(int  reinit);

  // Load/save settings
  STDMETHODIMP get_params(SpdiferParams *params);
  STDMETHODIMP set_params(SpdiferParams *params);
  STDMETHODIMP load_params(Config *config);
  STDMETHODIMP save_params(Config *config);
};

#endif
