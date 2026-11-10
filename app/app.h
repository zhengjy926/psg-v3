/**
 *******************************************************************************
 * @copyright : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
 * @file      : app.h
 * @author    : ZJY
 * @version   : V1.0
 * @date      : 2026-11-06
 * @brief     : 
 * @attention : None
 *******************************************************************************
 * @history   :
 *       V1.0 : 初始版本
 *******************************************************************************
 */

#ifndef APP_H_
#define APP_H_

#include <logging/log.h>
#include "../service/heartbeat/heartbeat.h"

struct app_config {
	struct heartbeat_config heartbeat;
};

int app_init(const struct app_config *cfg);
void app_step(void);

#endif /* PSG_APP_APP_H_ */
