/* Bug: PIPS prettyprinter eliminates significant SPACE characters. */

int main()
{
    int a=0;
    int b=0;

    // This does not work
    return 0+ ++a ;
    // This works
    return 0- ++a ;
    // This does not work
    return a++ + ++b;
    // This does not work
    return a++ + 0;

    // This does not work
    return 0- --a ;
    // This works
    return 0+ --a ;
    // This does not work
    return a-- - --b;
    // This does not work
    return a-- - 0;
}
