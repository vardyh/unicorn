#define PORT_FROM_QEMU_LATEST
#define TCG_CT_NEWREG 0x20

static const TCGTargetOpDef *tcg_target_op_def(TCGOpcode);
static const char *target_parse_constraint_new(TCGArgConstraint *ct,
                                           const char *ct_str, TCGType type);
static void sort_constraints(TCGOpDef *def, int start, int n);

static void process_op_defs(TCGContext *s)
{
    TCGOpcode op;

    for (op = 0; op < NB_OPS; op++) {
        TCGOpDef *def = &s->tcg_op_defs[op];
        const TCGTargetOpDef *tdefs;
        TCGType type;
        int i, nb_args;

        if (def->flags & TCG_OPF_NOT_PRESENT) {
            continue;
        }

        nb_args = def->nb_iargs + def->nb_oargs;
        if (nb_args == 0) {
            continue;
        }

        tdefs = tcg_target_op_def(op);
        /* Missing TCGTargetOpDef entry. */
        tcg_debug_assert(tdefs != NULL);

        type = (def->flags & TCG_OPF_64BIT ? TCG_TYPE_I64 : TCG_TYPE_I32);
        for (i = 0; i < nb_args; i++) {
            const char *ct_str = tdefs->args_ct_str[i];
            /* Incomplete TCGTargetOpDef entry. */
            tcg_debug_assert(ct_str != NULL);

            def->args_ct[i].u.regs = 0;
            def->args_ct[i].ct = 0;
            while (*ct_str != '\0') {
                switch(*ct_str) {
                case '0' ... '9':
                    {
                        int oarg = *ct_str - '0';
                        tcg_debug_assert(ct_str == tdefs->args_ct_str[i]);
                        tcg_debug_assert(oarg < def->nb_oargs);
                        tcg_debug_assert(def->args_ct[oarg].ct & TCG_CT_REG);
                        /* TCG_CT_ALIAS is for the output arguments.
                           The input is tagged with TCG_CT_IALIAS. */
                        def->args_ct[i] = def->args_ct[oarg];
                        def->args_ct[oarg].ct |= TCG_CT_ALIAS;
                        def->args_ct[oarg].alias_index = i;
                        def->args_ct[i].ct |= TCG_CT_IALIAS;
                        def->args_ct[i].alias_index = oarg;
                    }
                    ct_str++;
                    break;
                case '&':
                    def->args_ct[i].ct |= TCG_CT_NEWREG;
                    ct_str++;
                    break;
                case 'i':
                    def->args_ct[i].ct |= TCG_CT_CONST;
                    ct_str++;
                    break;
                default:
                    ct_str = target_parse_constraint_new(&def->args_ct[i],
                                                     ct_str, type);
                    /* Typo in TCGTargetOpDef constraint. */
                    tcg_debug_assert(ct_str != NULL);
                }
            }
        }

        /* TCGTargetOpDef entry with too much information? */
        tcg_debug_assert(i == TCG_MAX_OP_ARGS || tdefs->args_ct_str[i] == NULL);

        /* sort the constraints (XXX: this is just an heuristic) */
        sort_constraints(def, 0, def->nb_oargs);
        sort_constraints(def, def->nb_oargs, def->nb_iargs);
    }
}


static inline void tcg_out_reloc_qemu(TCGContext *s, tcg_insn_unit *code_ptr, int type,
                          TCGLabel *l, intptr_t addend)
{
    tcg_out_reloc(s, code_ptr, type, l->label_index, addend);
}
