enum
{
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};

int main()
{
  int i01, i02, i03, i04, i05, i06, i07, i08, i09, i10, i11, i12;
  int t;

  i01 = _ISupper;
  i02 = _ISlower;
  i03 = _ISalpha;
  i04 = _ISdigit;
  i05 = _ISxdigit;
  i06 = _ISspace;
  i07 = _ISprint;
  i08 = _ISgraph;
  i09 = _ISblank;
  i10 = _IScntrl;
  i11 = _ISpunct;
  i12 = _ISalnum;

  t = i01 + i02 + i03 + i04 + i05 + i06 + i07 + i08 + i09 + i10 + i11 + i12;
  return 0;
}
