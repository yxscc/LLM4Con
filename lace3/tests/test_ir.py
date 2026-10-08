from lace3.ir.module import load_program

from .conftest import line_of


def test_function_linkage_and_source_location(mini_ll):
    prog = load_program([mini_ll])
    setup = prog.function("demo_setup")
    assert setup.linkage == "external"
    assert setup.loc.file.endswith("mini_ops.c")
    assert setup.loc.line == line_of("void demo_setup(")
    assert prog.function("demo_open").linkage == "internal"


def test_direct_calls_carry_callee_site_and_function_pointer_args(mini_ll):
    prog = load_program([mini_ll])
    calls = {c.callee: c for c in prog.function("demo_setup").calls}
    assert set(calls) >= {"init_timer_key", "shared_helper"}
    timer = calls["init_timer_key"]
    assert timer.loc.line == line_of("init_timer_key(timer, demo_timer_fn")
    assert timer.fn_args == [(1, "demo_timer_fn")]


def test_call_graph_resolves_only_defined_callees(mini_ll):
    prog = load_program([mini_ll])
    callees = {f.name for f in prog.callees(prog.function("demo_setup"))}
    assert callees == {"shared_helper"}


def test_global_initializer_refs_name_the_struct_field(mini_ll):
    prog = load_program([mini_ll])
    refs = {(r.global_name, r.path, r.function) for r in prog.global_refs()}
    assert ("demo_fops", "open", "demo_open") in refs
    assert ("demo_fops", "release", "demo_release") in refs
    assert ("demo_ops_table", "[0].open", "demo_open") in refs
    assert ("demo_ops_table", "[1].release", "demo_ioctl") in refs
    assert ("__ksymtab_demo_exported", "", "demo_exported") in refs


def test_global_ref_reports_the_struct_that_owns_the_slot(mini_ll):
    prog = load_program([mini_ll])
    ref = next(r for r in prog.global_refs()
               if r.global_name == "demo_ops_table" and r.function == "demo_ioctl")
    assert ref.owner_struct == "demo_ops"


def test_store_through_anonymous_member_is_named_from_the_enclosing_struct(mini_ll):
    prog = load_program([mini_ll])
    st = prog.function("demo_set_destructor").fn_stores[0]
    assert (st.struct, st.field) == ("demo_buf", "destructor")


def test_store_into_global_struct_field_names_global_and_field(mini_ll):
    prog = load_program([mini_ll])
    st = prog.function("demo_register_notifier").fn_stores[0]
    assert st.dest == "field"
    assert (st.struct, st.field, st.target) == ("demo_notifier", "call", "demo_nb")


def test_store_into_function_pointer_array_member_marks_variable_index(mini_ll):
    prog = load_program([mini_ll])
    by_line = {s.loc.line: s for s in prog.function("demo_add_mod").fn_stores}
    var = by_line[line_of("m->modfunc[i] =")]
    const = by_line[line_of("m->modfunc[2] =")]
    assert (var.struct, var.field) == ("demo_mod", "modfunc[*]")
    assert (const.struct, const.field) == ("demo_mod", "modfunc[2]")


def test_call_inside_always_inline_helper_is_located_at_the_outer_call(mini_ll):
    prog = load_program([mini_ll])
    call = next(c for c in prog.function("demo_arm").calls if c.callee == "init_timer_key")
    assert call.loc.line == line_of("demo_timer_setup(t);")
    assert call.loc.via.endswith(f"mini_ops.c:{line_of('init_timer_key(t, demo_timer_fn')}")


def test_multiline_switch_and_asm_goto_keep_instructions_aligned_with_text(mini_ll):
    prog = load_program([mini_ll])
    st = prog.function("demo_dispatch").fn_stores[0]
    assert (st.struct, st.field) == ("work_struct", "func")
    assert st.loc.line == line_of("d->work.func = demo_work_fn;", nth=2)


def test_store_of_function_address_names_the_destination_field(mini_ll):
    prog = load_program([mini_ll])
    stores = prog.function("demo_setup").fn_stores
    assert len(stores) == 1
    st = stores[0]
    assert st.function == "demo_work_fn"
    assert (st.struct, st.field) == ("work_struct", "func")
    assert st.loc.line == line_of("d->work.func = demo_work_fn;")
