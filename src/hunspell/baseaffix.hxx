#ifndef _BASEAFF_HXX_
#define _BASEAFF_HXX_

#include "hunvisapi.h"
#include <string>

class LIBHUNSPELL_DLL_EXPORTED AffEntry
{
private:
    AffEntry(const AffEntry&);
    AffEntry& operator = (const AffEntry&);
protected:
    AffEntry()
      : strip(NULL)
      , stripl(0)
      , numconds(0)
      , opts(0)
      , aflag(0)
      , morphcode(0)
      , contclass(NULL)
      , contclasslen(0)
    {
    }
    std::string    appnd;
    char *         strip;
    unsigned char  stripl;
    char           numconds;
    char           opts;
    unsigned short aflag;
    union {
        char       conds[MAXCONDLEN];
        struct {
            char   conds1[MAXCONDLEN_1];
            char * conds2;
        } l;
    } c;
    char *           morphcode;
    unsigned short * contclass;
    short            contclasslen;
};

#endif
