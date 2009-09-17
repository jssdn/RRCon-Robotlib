isr 
// Should exist here?
int gpio_mapall()
{
    int res;

    if( (res = mapio_region(&led4_base, LED4_BASE, LED4_END)) < 0 ) 
        return res; 
    printf("led4_base 0x%x\n",led4_base);
    *(led4_base + GPIO_TRISTATE_OFFSET) = 0x00 ; // configured as outputs

    if( (res = mapio_region(&dirled_base, DIRLED_BASE,DIRLED_END)) < 0 ) 
        return res; 

    *(dirled_base + GPIO_TRISTATE_OFFSET) = 0x00 ; // configured as outputs

    if( (res = mapio_region(&buttons_base, BUTTONS_BASE,BUTTONS_END)) < 0 ) 
        return res; 

    *(buttons_base + GPIO_TRISTATE_OFFSET) = 0xff ; // configured as inputs

    if( (res = mapio_region(&bumpers_base, BUMPERS_BASE,BUMPERS_END)) < 0 ) 
        return res; 

    *(bumpers_base + GPIO_TRISTATE_OFFSET) = 0xff ; // configured as inputs

    return 0; 
}

int gpio_unmapall()
{
    int res;

    if( (res = unmapio_region(&led4_base,LED4_BASE,LED4_END)) < 0 )
            return res;		

    if( (res = unmapio_region(&dirled_base,DIRLED_BASE,DIRLED_END)) < 0 )
            return res;		

    if( (res = unmapio_region(&buttons_base, BUTTONS_BASE,BUTTONS_END)) < 0 )
            return res;		

    return 0; 
}

/*inline void write_led4(uint8_t val)
{
    *led4_base = val & 0x0f;
}

inline void write_led_dir(uint8_t val)
{
    *dirled_base = val & 0x1f; 
}

// Polling
inline uint8_t read_buttons()
{
    return *buttons_base;
}

inline uint8_t read_bumpers()
{
  return *bumpers_base; */
}

// enum{
//     LEFT = 0x01, 
//     RIGHT = 0x02,
//     UP = 0x03,
//     DOWN = 0x04,
//     CENTER = 0x05
// } directions; 

int map_gpio();

int unmap_gpio();

inline void write_led4(uint8_t val);

inline void write_led_dir(uint8_t val);

// Polling
inline uint8_t read_buttons();
