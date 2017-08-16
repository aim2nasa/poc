{
  "targets": [
    {
      "target_name": "hsmproxy",
      "sources": [ "hsm.cpp" ],
      "libraries": [ "-L../","-lhsmproxy", "-L/usr/local/lib/softhsm","-lsofthsm2" ],
      "include_dirs" : [ "<!(node -e \"require('nan')\")", "../common" ]
    }
  ]
}
