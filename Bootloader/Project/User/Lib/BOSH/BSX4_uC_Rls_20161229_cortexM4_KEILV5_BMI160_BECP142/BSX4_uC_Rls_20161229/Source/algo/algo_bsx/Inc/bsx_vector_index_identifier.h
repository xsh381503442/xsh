 /*
 * $license$
 */

/*!@addtogroup bsx_external
 * @{*/

#ifndef __BSX_VECTOR_INDEX_IDENTIFIER_H__
#define __BSX_VECTOR_INDEX_IDENTIFIER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*! @brief BSX vector indices */
typedef enum
{
    BSX_ID_ROTATION_QUATERNION_X            = (0),
    BSX_ID_ROTATION_QUATERNION_Y            = (1),
    BSX_ID_ROTATION_QUATERNION_Z            = (2),
    BSX_ID_ROTATION_QUATERNION_W            = (3),
    BSX_ID_ROTATION_STATUS                  = (4),
    BSX_ID_ORIENTATION_EULERANGLE_YAW       = (0),
    BSX_ID_ORIENTATION_EULERANGLE_PITCH     = (1),
    BSX_ID_ORIENTATION_EULERANGLE_ROLL      = (2),
    BSX_ID_ORIENTATION_EULERANGLE_HEADING   = (3),
    BSX_ID_ORIENTATION_STATUS               = (4),
    BSX_ID_MOTION_VECTOR_X                  = (0),
    BSX_ID_MOTION_VECTOR_Y                  = (1),
    BSX_ID_MOTION_VECTOR_Z                  = (2),
    BSX_ID_MOTION_STATUS                    = (3)
} bsx_vector_index_identifier_t;


#ifdef __cplusplus
}
#endif

#endif /*__BSX_VECTOR_INDEX_IDENTIFIER_H__*/
/*! @}*/
