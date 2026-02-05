#ifndef MLX90614_H
#define MLX90614_H

// 外部公開関数
extern int   is_ready_mlx90614();
extern void  mlx90614_init();
extern int   mlx90614_enter_sleep();
extern int   mlx90614_exit_sleep();
extern float mlx90614_read_env_temp();
extern float mlx90614_read_obj_temp();

#endif // MLX90614_H