#ifndef _DEF_H
#define _DEF_H

#define EPS 0.1f

#define M_PIf		3.14159265358979323846f	/* pi */
#define M_PI_2f	1.57079632679489661923f	/* pi/2 */
#define M_PI_4f	0.78539816339744830962f	/* pi/4 */
#define M_PI		3.14159265358979323846	/* pi */
#define M_PI_2		1.57079632679489661923	/* pi/2 */
#define M_PI_4		0.78539816339744830962	/* pi/4 */

#define WWIDTH  800
#define WHEIGHT 600

#define FLOOR_Y (-100.0f)
#define WIND_RESISTANCE (0.0f)
#define GRAVITY (9.8f)

#define PLAYER_ID (1)
#define ANOTHER_PLAYER_ID (0)
#define AIM_DISTANCE (20.0f)

#define MAP_SIZE (10)

#define OBB_SHOW 1          // OBB是否显示

constexpr glm::vec3 CAMERA_TO_PLAYER_OFFSET = glm::vec3(0.0f, 0.8f, 0.0f);

#endif
