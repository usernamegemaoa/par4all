from __future__ import with_statement # this is to work with python2.5
import terapyps
from pyps import workspace
workspace.delete("add")
with terapyps.workspace("add.c", name="add", deleteOnClose=False,recoverInclude=False) as w:
    for f in w.fun:
        f.terapix_code_generation(debug=True)
#    w.compile(terapyps.Maker())
