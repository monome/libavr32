#include "interrupt.h"
#include "conf_tc_irq.h"
#include "interrupts.h"
#include "types.h"

u8 irqs_pause( void ) {
  u8 irq_flags = 0;
  if (cpu_irq_level_is_enabled(SYS_IRQ_PRIORITY)) {
    irq_flags |= 1 << SYS_IRQ_PRIORITY;
    cpu_irq_disable_level(SYS_IRQ_PRIORITY);
  }
  if (cpu_irq_level_is_enabled(UI_IRQ_PRIORITY)) {
    irq_flags |= 1 << UI_IRQ_PRIORITY;
    cpu_irq_disable_level(UI_IRQ_PRIORITY);
  }
  if (cpu_irq_level_is_enabled(APP_TC_IRQ_PRIORITY)) {
    irq_flags |= 1 << APP_TC_IRQ_PRIORITY;
    cpu_irq_disable_level(APP_TC_IRQ_PRIORITY);
  }
  return irq_flags;
}

void irqs_resume( u8 irq_flags ) {
  if (irq_flags & (1 << APP_TC_IRQ_PRIORITY)) {
    cpu_irq_enable_level(APP_TC_IRQ_PRIORITY);
  }
  if (irq_flags & (1 << UI_IRQ_PRIORITY)) {
    cpu_irq_enable_level(UI_IRQ_PRIORITY);
  }
  if (irq_flags & (1 << SYS_IRQ_PRIORITY)) {
    cpu_irq_enable_level(SYS_IRQ_PRIORITY);
  }
}
