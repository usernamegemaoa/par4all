/* $Id$
 */

/* some useful SHORTHANDS for EFFECT:
 */
#define effect_entity(e) reference_variable(effect_reference(e))
#define effect_action_tag(eff) action_tag(effect_action(eff))
#define effect_approximation_tag(eff) \
	approximation_tag(effect_approximation(eff))

#define effect_scalar_p(eff) entity_scalar_p(effect_entity(eff))
#define effect_read_p(eff) (action_tag(effect_action(eff))==is_action_read)
#define effect_write_p(eff) (action_tag(effect_action(eff))==is_action_write)
#define effect_may_p(eff) \
        (approximation_tag(effect_approximation(eff)) == is_approximation_may)
#define effect_must_p(eff) \
        (approximation_tag(effect_approximation(eff)) == is_approximation_must)
#define effect_exact_p(eff) \
        (approximation_tag(effect_approximation(eff)) ==is_approximation_exact)

/* some string constants for prettyprints...
 */
#define ACTION_UNDEFINED 	string_undefined
#define ACTION_READ 		"R"
#define ACTION_WRITE 		"W"
#define ACTION_IN    		"IN"
#define ACTION_OUT		"OUT"
#define ACTION_COPYIN		"COPYIN"
#define ACTION_COPYOUT		"COPYOUT"
#define ACTION_PRIVATE		"PRIVATE"

/* prettyprint function types:
 */
typedef text (*generic_text_function)(list /* of effect */);
typedef void (*generic_prettyprint_function)(list /* of effect */);
typedef void (*generic_attachment_function)(text);

/* For COMPATIBILITY purpose only - DO NOT USE anymore
 */
#define effect_variable(e) reference_variable(effect_reference(e))

/* end of effects-generic-local.h
 */
