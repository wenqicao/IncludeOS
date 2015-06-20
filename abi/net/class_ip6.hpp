#ifndef CLASS_IP6_HPP
#define CLASS_IP6_HPP

#include <delegate>
#include <net/inet.hpp>
#include <net/class_ethernet.hpp>
#include <net/ip6/icmp6.hpp>
#include <net/ip6/udp6.hpp>

#include <iostream>
#include <string>
#include <map>
#include <x86intrin.h>

#include <stdint.h>
#include <assert.h>

namespace net
{
  class Packet;
  
  /** IP6 layer skeleton */
  class IP6
  {
  public:
    /** Known transport layer protocols. */
    enum proto
    {
      PROTO_HOPOPT =  0, // IPv6 hop-by-hop
      
      PROTO_ICMPv4 =  1,
      PROTO_TCP    =  6,
      PROTO_UDP    = 17,
      
      PROTO_ICMPv6 = 58, // IPv6 ICMP
      PROTO_NoNext = 59, // no next-header
      PROTO_OPTSv6 = 60, // dest options
    };
    
    /** Handle IPv6 packet. */
    int bottom(std::shared_ptr<Packet>& pckt);
    
    struct addr
    {
      // constructors
      addr()
        : i64{0, 0} {}
      addr(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
        : i32{a, b, c, d} {}
      addr(uint64_t top, uint64_t bot)
        : i64{top, bot} {}
      //addr(__m128i address)
      //  : i128(address) {}
      // copy-constructor
      addr(const addr& a)
      {
        printf("IPv6::addr copy constructor %s\n", a.to_string().c_str());
        //i128 = a.i128;
        this->i64[0] = a.i64[0];
        this->i64[1] = a.i64[1];
      }
      // move constructor
      addr& operator= (const addr& a)
      {
        printf("IPv6::addr move constructor %s\n", a.to_string().c_str());
        //i128 = a.i128;
        this->i64[0] = a.i64[0];
        this->i64[1] = a.i64[1];
        return *this;
      }
      // returns this IPv6 address as a string
      std::string to_string() const;
      
      union
      {
        //__m128i  i128;
        uint64_t i64[2];
        uint32_t i32[4];
        uint16_t i16[8];
        uint8_t  i8[16];
      };
      
    } __attribute__((aligned(16)));
    
    #pragma pack(push, 1)
    class header
    {
    public:
      uint8_t version() const
      {
        return (scanline[0] & 0xF0) >> 4;
      }
      uint8_t tclass() const
      {
        return ((scanline[0] & 0xF000) >> 12) + 
                (scanline[0] & 0xF);
      }
      
      uint16_t size() const
      {
        return ((scanline[1] & 0x00FF) << 8) +
               ((scanline[1] & 0xFF00) >> 8);
      }
      
      uint8_t next() const
      {
        return (scanline[1] >> 16) & 0xFF;
      }
      uint8_t hoplimit() const
      {
        return (scanline[1] >> 24) & 0xFF;
      }
      
      // 128-bit is probably not good as "by value"
      const addr& source() const
      {
        return src;
      }
      const addr& dest() const
      {
        return dst;
      }
      
    private:
      uint32_t scanline[2];
      addr     src;
      addr     dst;
    };
    
    struct options_header
    {
      uint8_t  next_header;
      uint8_t  hdr_ext_len;
      uint16_t opt_1;
      uint32_t opt_2;
      
      uint8_t next() const
      {
        return next_header;
      }
      uint8_t size() const
      {
        return sizeof(options_header) + hdr_ext_len;
      }
      uint8_t extended() const
      {
        return hdr_ext_len;
      }
    };
    #pragma pack(pop)
    
    struct full_header
    {
      Ethernet::header eth_hdr;
      IP6::header      ip6_hdr;
    };
    
    /** Constructor. Requires ethernet to latch on to. */
    IP6(const addr& local);
    
    const IP6::addr& getIP() const
    {
      return local;
    }
    
    uint8_t parse6(uint8_t*& reader, uint8_t next);
    
    static std::string protocol_name(uint8_t protocol)
    {
      switch (protocol)
      {
      case PROTO_HOPOPT:
        return "IPv6 Hop-By-Hop (0)";
        
      case PROTO_TCP:
        return "TCPv6 (6)";
      case PROTO_UDP:
        return "UDPv6 (17)";
        
      case PROTO_ICMPv6:
        return "ICMPv6 (58)";
      case PROTO_NoNext:
        return "No next header (59)";
      case PROTO_OPTSv6:
        return "IPv6 destination options (60)";
        
      default:
        return "Unknown: " + std::to_string(protocol);
      }
    }
    
    // modify upstream handlers
    inline void set_handler(uint8_t proto, upstream& handler)
    {
      proto_handlers[proto] = handler;
    }
    
  private:
    addr local;
    
    /** Downstream: Linklayer output delegate */
    downstream _linklayer_out;
    
    /** Upstream delegates */
    std::map<uint8_t, upstream> proto_handlers;
  };
  
  inline std::ostream& operator<< (std::ostream& out, const IP6::addr& ip)
  {
    return out << ip.to_string();
  }
  
} // namespace net

#endif
