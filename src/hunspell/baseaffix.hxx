#ifndef _BASEAFF_HXX_
#define _BASEAFF_HXX_

#include "hunvisapi.h"

class LIBHUNSPELL_DLL_EXPORTED AffEntry
{
private:
    AffEntry(const AffEntry&);
    AffEntry& operator = (const AffEntry&);
protected:
    AffEntry()
      : appnd(NULL)
      , strip(NULL)
      , appndl(0)
      , stripl(0)
      , numconds(0)
      , opts(0)
      , aflag(0)
      , morphcode(0)
      , contclass(NULL)
      , contclasslen(0)
    {
    }
    char *         appnd;
    char *         strip;
    unsigned char  appndl;
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
