/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.5 */

#ifndef PB_FEEDBACK_PB_H_INCLUDED
#define PB_FEEDBACK_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _Feedback { 
    uint32_t id; 
    uint32_t status; 
    float battery; 
    float encoder1; 
    float encoder2; 
    float encoder3; 
    float encoder4; 
} Feedback;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define Feedback_init_default                    {0, 0, 0, 0, 0, 0, 0}
#define Feedback_init_zero                       {0, 0, 0, 0, 0, 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define Feedback_id_tag                          1
#define Feedback_status_tag                      2
#define Feedback_battery_tag                     3
#define Feedback_encoder1_tag                    4
#define Feedback_encoder2_tag                    5
#define Feedback_encoder3_tag                    6
#define Feedback_encoder4_tag                    7

/* Struct field encoding specification for nanopb */
#define Feedback_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, UINT32,   id,                1) \
X(a, STATIC,   REQUIRED, UINT32,   status,            2) \
X(a, STATIC,   REQUIRED, FLOAT,    battery,           3) \
X(a, STATIC,   REQUIRED, FLOAT,    encoder1,          4) \
X(a, STATIC,   REQUIRED, FLOAT,    encoder2,          5) \
X(a, STATIC,   REQUIRED, FLOAT,    encoder3,          6) \
X(a, STATIC,   REQUIRED, FLOAT,    encoder4,          7)
#define Feedback_CALLBACK NULL
#define Feedback_DEFAULT NULL

extern const pb_msgdesc_t Feedback_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define Feedback_fields &Feedback_msg

/* Maximum encoded size of messages (where known) */
#define Feedback_size                            37

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif