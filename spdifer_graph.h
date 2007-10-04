#ifndef SPDIFER_GRAPH_H
#define SPDIFER_GRAPH_H

#include "filter_graph.h"
#include "filters\spdifer.h"
#include "filters\demux.h"
#include "filters\convert.h"

class SpdiferGraph : public FilterGraph
{
protected:
  enum state_t 
  { 
    state_demux,
    state_spdifer,
    state_conv1,
    state_conv2,
  };            

  Demux demux;
  Spdifer spdifer;
  Converter conv1;
  Converter conv2;

public:
  SpdiferGraph(): FilterGraph(-1), conv1(2048), conv2(2048)
  {
    conv1.set_format(FORMAT_LINEAR);
    conv2.set_format(FORMAT_PCM16);
  }

  /////////////////////////////////////////////////////////
  // Filter

  void reset()
  {
    demux.reset();
    spdifer.reset();
    conv1.reset();
    conv2.reset();
    FilterGraph::reset();
  }

  /////////////////////////////////////////////////////////
  // Spdifer interface

  int        get_dts_mode()                  const { return spdifer.get_dts_mode();     }
  void       set_dts_mode(int dts_mode)            { spdifer.set_dts_mode(dts_mode);    }
  int        get_dts_conv()                  const { return spdifer.get_dts_conv();     }
  void       set_dts_conv(int dts_conv)            { spdifer.set_dts_conv(dts_conv);    }
  int        get_frames()                    const { return spdifer.get_frames();       }
  int        get_errors()                    const { return spdifer.get_errors();       }
  size_t     get_info(char *buf, size_t len) const { return spdifer.get_info(buf, len); }
  HeaderInfo header_info()                   const { return spdifer.header_info();      }

  /////////////////////////////////////////////////////////
  // FilterGraph

  virtual const char *get_name(int node) const
  {
    switch (node)
    {
      case state_demux:     return "Demuxer";
      case state_spdifer:   return "Spdifer";
      case state_conv1:     return "Convert PCM->internal";
      case state_conv2:     return "Convert internal->PCM";
    }
    return 0;
  }

  virtual Filter *init_filter(int node, Speakers spk)
  {
    switch (node)
    {
      case state_demux:     return &demux;
      case state_spdifer:   return &spdifer;
      case state_conv1:     return &conv1;
      case state_conv2:     return &conv2;
    }
    return 0;
  }

  virtual int get_next(int node, Speakers spk) const
  {
    switch (node)
    {
      case node_start:
        if (demux.query_input(spk)) return state_demux;
        if (spdifer.query_input(spk)) return state_spdifer;
        if (conv1.query_input(spk)) return state_conv1;
        return node_err;

      case state_demux:
        if (spdifer.query_input(spk)) return state_spdifer;
        if (conv1.query_input(spk)) return state_conv1;
        return node_err;

      case state_spdifer:
        return node_end;

      case state_conv1:
        return state_conv2;

      case state_conv2:
        return node_end;
    }

    return node_end;
  }
};


#endif
