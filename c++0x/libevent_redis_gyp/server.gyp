{
  'targets': [
    {
      'target_name': 'server',
      'type': 'executable',
      'sources': [ 'server.cc' ],
      'include_dirs': [ '/usr/local/include/hiredis' ],
      'conditions': [
          ['OS == "linux"', {
            'cflags': [
              '-g',
              '-std=c++11'
            ],
            'link_settings': {
              'libraries': [
                '-L/usr/local/lib',
                '-levent',
                '-lhiredis',
                '-lpthread',
                '-lrt'
              ]},
          }],
        ]
    },
  ],
}
