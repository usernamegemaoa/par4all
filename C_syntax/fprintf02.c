//#include <stdio.h>

void fprintf02()
{
  //FILE * p_trace;
  int symbolCount = 0;
  struct se_struct {
    char * tracestring;
    int value1;
  } * se;

  //fprintf(p_trace, "@%-6d %-63s (%3d)\n",symbolCount++, se->tracestring, se->value1);
  foo("@%-6d %-63s (%3d)\n",symbolCount++, se->tracestring, se->value1);
}
