#include <video.h>
#include <touchscreen.h>
#include <display.h>

char *get_command(struct resource *src)
{
    int get_x, get_y;
    int status;
    static int flag = 1;
    int pos, retval = 0;
    char buffer[100];
    while (1)
    {
        status = get_touchscreen_coordinate(src->touch_fd, &get_x, &get_y);
        if (status == TOUCHSCREEN_RELEASE_STATUS)
        {
            if (judge_touch_area(get_x, get_y, 260, 340, 410, 479) == 0) //暂停判定
            {
                flag = -flag;

                printf("flag=%d\n", flag);

                display_touch_bar(src, flag);
                sprintf(buffer, "pause\n");
                break;
            }
            else if (judge_touch_area(get_x, get_y, 500, 575, 410, 479) == 0) //音量+判定
            {
                sprintf(buffer, "volume +10\n");
                break;
            }
            else if (judge_touch_area(get_x, get_y, 60, 100, 410, 479) == 0) //音量-判定
            {
                sprintf(buffer, "volume -10\n");
                break;
            }
            else if (judge_touch_area(get_x, get_y, 375, 455, 410, 479) == 0) //下一个视频
            {
                if (flag < 0)
                {
                    break;
                }

                sprintf(buffer, "quit\n");
                break;
            }
            else if (judge_touch_area(get_x, get_y, 145, 230, 410, 479) == 0) //上一个视频
            {
                if (flag < 0)
                {
                    break;
                }
                src->video_order -= 2;
                if (src->video_order < 0)
                {
                    src->video_order = 0;
                }

                sprintf(buffer, "quit\n");
                break;
            }
            // else if (judge_touch_area(get_x, get_y, 0, 600, 350, 405) == 0) //滑动进度条
            // {
            //     pos = ctrl_progress_bar(src);
            //     sprintf(buffer, "seek %d 1\n", pos);
            //     break;
            // }
        }
        else if (status == TOUCHSCREEN_TOUCH_STATUS && judge_touch_area(get_x, get_y, 600, 799, 0, 480) == 0)
        {
            retval = select_menu(src, get_x, get_y);
            if (retval == 1)
            {
                sprintf(buffer, "quit\n");
                flag = 1;
            }
            break;
        }
        else if (status == TOUCHSCREEN_TOUCH_STATUS && (judge_touch_area(get_x, get_y, 0, 600, 350, 405) == 0))
        {
            pos = ctrl_progress_bar(src);
            sprintf(buffer, "seek %d 1\n", pos);
            break;
        }
    }
    return buffer;
}

int show_percent(struct resource *src, char *buf) //显示百分比
{
    int pos = 0;
    float time_pogress;
    static float video_length = 0;
    if (strncmp(buf, "ANS_TIME_POSITION", strlen("ANS_TIME_POSITION")) == 0)
    {
        sscanf(buf, "ANS_TIME_POSITION=%f", &time_pogress);
        pos = (time_pogress / video_length) * 100;
        progress_bar(src, pos, 0, 380, 600, 20);
    }
    else if (strncmp(buf, "ANS_LENGTH", strlen("ANS_LENGTH")) == 0)
    {
        sscanf(buf, "ANS_LENGTH=%f", &video_length);
    }

    return pos;
}

int progress_bar(struct resource *src, int rate, int start_x, int start_y, int width, int height)
{
    int x = 0, y = 0;

    //白色空条
    for (x = start_x; x < (start_x + width); x++)
    {
        for (y = start_y; y < (start_y + height); y++)
        {
            // display_framebuffer_point(int x, int y, unsigned int color, unsigned int *framebuffer_addr)

            // src.framebuffer_addr[y*FRAMEBUFFER_WIDE+x] = WHITE;

            display_framebuffer_point(x, y, WHITE, src->framebuffer_addr);
        }
    }
    //绿色已读
    for (x = start_x; x < ((start_x + width) * rate / 100); x++)
    {
        for (y = start_y; y < (start_y + height); y++)
        {
            // src.framebuffer_addr[y*FRAMEBUFFER_WIDE + x] = GREEN;

            display_framebuffer_point(x, y, GREEN, src->framebuffer_addr);
        }
    }
    return 0;
}

int ctrl_progress_bar(struct resource *src)
{
    int get_x, get_y;
    int status;
    int ts_x, ts_y;
    float pos;
    while (1)
    {
        status = get_touchscreen_coordinate(src->touch_fd, &ts_x, &ts_y);

        if (status == TOUCHSCREEN_GET_COORDINATE)
        {
            get_x = ts_x;
            pos = (get_x * 1.0 / 600) * 100;
            progress_bar(src, (int)pos, 0, 380, 600, 20);
        }
        else if (status == TOUCHSCREEN_RELEASE_STATUS)
        {
            get_x = ts_x;
            pos = (get_x * 1.0 / 600) * 100;
            break;
        }
    }
    return (int)pos;
}

int select_menu(struct resource *src, int init_x, int init_y)
{
    int ts_x, ts_y, status;
    int ture_y;
    static int offset_y = 0;
    int start_x, start_y, to_x, to_y;
    int get_x, get_y, end_x, end_y, diff_y;
    // struct resource *src = (struct resource *)(arg);
    // display_menu(src, 0, 0);

    start_x = init_x;
    start_y = init_y;
    to_x = init_x;
    to_y = init_y;
    printf("1\n");
    while (1)
    {
        status = get_touchscreen_coordinate(src->touch_fd, &ts_x, &ts_y);
        // if ((ts_x > 600) && status == TOUCHSCREEN_TOUCH_STATUS)
        // {
        //     start_x = ts_x;
        //     start_y = ts_y;
        //     to_x = ts_x;
        //     to_y = ts_y;
        // }
        if (status == TOUCHSCREEN_GET_COORDINATE)
        {
            get_x = ts_x;
            get_y = ts_y;
            diff_y = abs(start_y - end_y);
            if (diff_y < 10)
            {
                continue;
            }
            else
            {
                offset_y += (to_y - get_y);
                if (offset_y < 0)
                {
                    offset_y = 0;
                    display_menu(src, 0, offset_y);
                }
                else if (offset_y > (src->buffer_height - 481))
                {
                    offset_y = src->buffer_height - 481;
                    display_menu(src, 0, offset_y);
                }
                else
                {
                    display_menu(src, 0, offset_y);
                }
                to_y = ts_y;
            }
        }
        else if (status == TOUCHSCREEN_RELEASE_STATUS)
        {
            end_x = ts_x;
            end_y = ts_y;
            diff_y = abs(start_y - end_y);
            if (diff_y < 10)
            {
                //选择不同影片
                ture_y = end_y + offset_y; //算出真实显示的y坐标
                printf("当前选中的坐标为：end_x = %d,end_y = %d, true_y = %d\n", end_x, end_y, ture_y);
                printf("src.video_order == %d\n", src->video_order);
                int i;
                for (i = 0; i < src->video_sum; i++)
                {
                    printf("qq\n");
                    if ((judge_touch_area(end_x, ture_y, 615, 799, 30 + (i * 150), 150 + (i * 150)) == 0))
                    {
                        src->video_order = i;
                        printf("选中src.video == %d\n", src->video_order);
                        return 1;
                    }
                }
                printf("hello\n");
            }
            break;
        }
    }
    return 0;
}
