{
    "targets": [
        {
            'target_name': "faiss",
            'sources': [ "src/faiss.cc" ],
            'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"],
            'cflags!': [ '-fno-exceptions' ],
            'cflags_cc!': [ '-fno-exceptions' ],
            'conditions': [
                ["OS=='win'", {
                    "defines": [
                        "_HAS_EXCEPTIONS=1"
                    ],
                    "msvs_settings": {
                        "VCCLCompilerTool": {
                        "ExceptionHandling": 1
                        },
                    },
                }],
                ["OS=='mac'", {
                    'cflags+': ['-fvisibility=hidden'],
                    'xcode_settings': {
                        'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                    },
                }],
            ],
        }
    ]
}
