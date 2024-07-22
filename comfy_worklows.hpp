#include <nlohmann/json.hpp>

namespace comfy_workflows
{
    const nlohmann::json defaultWorkflow = R"(
        {
        "client_id": "",
        "prompt": {
            "3": {
                "class_type": "KSampler",
                "inputs": {
                    "cfg": 8,
                    "denoise": 1,
                    "latent_image": [
                        "5",
                        0
                    ],
                    "model": [
                        "4",
                        0
                    ],
                    "negative": [
                        "7",
                        0
                    ],
                    "positive": [
                        "6",
                        0
                    ],
                    "sampler_name": "euler",
                    "scheduler": "normal",
                    "seed": 156680208700286,
                    "steps": 20
                }
            },
            "4": {
                "class_type": "CheckpointLoaderSimple",
                "inputs": {
                    "ckpt_name": ""
                }
            },
            "5": {
                "class_type": "EmptyLatentImage",
                "inputs": {
                    "batch_size": 1,
                    "height": 512,
                    "width": 512
                }
            },
            "6": {
                "class_type": "CLIPTextEncode",
                "inputs": {
                    "clip": [
                        "4",
                        1
                    ],
                    "text": ""
                }
            },
            "7": {
                "class_type": "CLIPTextEncode",
                "inputs": {
                    "clip": [
                        "4",
                        1
                    ],
                    "text": ""
                }
            },
            "8": {
                "class_type": "VAEDecode",
                "inputs": {
                    "samples": [
                        "3",
                        0
                    ],
                    "vae": [
                        "4",
                        2
                    ]
                }
            },
            "9": {
                "class_type": "SaveImage",
                "inputs": {
                    "filename_prefix": "gen",
                    "images": [
                        "8",
                        0
                    ]
                }
            }
        }
    }
    )"_json;
nlohmann::json SD3Workflow = R"(
{
    "client_id": "1e316ee5e2894fbb896006ef822c894b",
    "prompt": {
        "6": {
            "inputs": {
            "text": "a female character with long, flowing hair",
            "clip": [
                "273",
                0
            ]
            },
            "class_type": "CLIPTextEncode"
        },
        "13": {
            "inputs": {
            "shift": 3,
            "model": [
                "4",
                0
            ]
            },
            "class_type": "ModelSamplingSD3"
        },
        "67": {
            "inputs": {
            "conditioning": [
                "7",
                0
            ]
            },
            "class_type": "ConditioningZeroOut"
        },
        "68": {
            "inputs": {
            "start": 0.1,
            "end": 1,
            "conditioning": [
                "67",
                0
            ]
            },
            "class_type": "ConditioningSetTimestepRange"
        },
        "69": {
            "inputs": {
            "conditioning_1": [
                "68",
                0
            ],
            "conditioning_2": [
                "70",
                0
            ]
            },
            "class_type": "ConditioningCombine"
        },
        "70": {
            "inputs": {
            "start": 0,
            "end": 0.1,
            "conditioning": [
                "7",
                0
            ]
            },
            "class_type": "ConditioningSetTimestepRange"
        },
        "7": {
            "inputs": {
            "text": "",
            "clip": [
                "273",
                0
            ]
            },
            "class_type": "CLIPTextEncode"
        },
        "135": {
            "inputs": {
            "width": 512,
            "height": 512,
            "batch_size": 1
            },
            "class_type": "EmptySD3LatentImage"
        },
        "231": {
            "inputs": {
            "samples": [
                "271",
                0
            ],
            "vae": [
                "4",
                2
            ]
            },
            "class_type": "VAEDecode"
        },
        "9": {
            "class_type": "SaveImage",
            "inputs": {
                "filename_prefix": "gen",
                "images": [
                    "231",
                    0
                ]
            }
        },
        "4": {
            "inputs": {
            "ckpt_name": "sd3_medium.safetensors"
            },
            "class_type": "CheckpointLoaderSimple"
        },
        "271": {
            "inputs": {
            "seed": 945512652412924,
            "steps": 28,
            "cfg": 7,
            "sampler_name": "dpmpp_2m",
            "scheduler": "sgm_uniform",
            "denoise": 1,
            "model": [
                "13",
                0
            ],
            "positive": [
                "6",
                0
            ],
            "negative": [
                "69",
                0
            ],
            "latent_image": [
                "135",
                0
            ]
            },
            "class_type": "KSampler"
        },
        "273": {
            "inputs": {
            "clip_name1": "clip_g.safetensors",
            "clip_name2": "clip_l.safetensors",
            "type": "sd3"
            },
            "class_type": "DualCLIPLoader"
        }    
    }
}
)"_json;
};

