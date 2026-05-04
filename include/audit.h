#ifndef LOOGAL_AUDIT_H
#define LOOGAL_AUDIT_H

void loogal_audit_event(const char *event, const char *status, const char *message);
void loogal_log(const char *event, const char *status, const char *message);

#endif
