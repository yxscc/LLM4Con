from lace3.entries.discover import discover_entries
from lace3.ir.module import load_program

from .conftest import line_of


def entries_by_name(mini_ll):
    return {e.function.name: e for e in discover_entries(load_program([mini_ll])).entries}


def facts(entry, kind):
    return [f for f in entry.facts if f.kind == kind]


def test_ops_table_slot_is_an_entry_with_global_and_field(mini_ll):
    e = entries_by_name(mini_ll)["demo_release"]
    (a,) = [f for f in facts(e, "A") if f.detail["global"] == "demo_fops"]
    assert a.detail["path"] == "release"
    assert a.detail["struct"] == "demo_ops"


def test_ops_table_array_element_slot_keeps_the_index(mini_ll):
    e = entries_by_name(mini_ll)["demo_ioctl"]
    (a,) = facts(e, "A")
    assert (a.detail["global"], a.detail["path"]) == ("demo_ops_table", "[1].release")


def test_callback_stored_into_a_field_is_an_entry_with_the_store_site(mini_ll):
    e = entries_by_name(mini_ll)["demo_work_fn"]
    sites = {(f.detail["struct"], f.detail["field"], f.detail["line"]) for f in facts(e, "S")}
    assert ("work_struct", "func", line_of("d->work.func = demo_work_fn;")) in sites


def test_callback_passed_to_an_undefined_callee_is_an_entry(mini_ll):
    e = entries_by_name(mini_ll)["demo_timer_fn"]
    b = facts(e, "B")
    assert {(f.detail["callee"], f.detail["arg"]) for f in b} == {("init_timer_key", 1)}


def test_syscall_wrapper_name_is_an_entry(mini_ll):
    assert facts(entries_by_name(mini_ll)["__do_sys_demo"], "C")


def test_exported_symbol_is_an_entry_even_when_the_marker_is_in_a_discard_section(mini_ll):
    es = entries_by_name(mini_ll)
    assert facts(es["demo_exported"], "E")
    assert facts(es["demo_exported_v7"], "E")


def test_reference_from_a_discarded_non_export_section_is_not_an_entry(mini_ll):
    assert "demo_discarded_fn" not in entries_by_name(mini_ll)


def test_external_function_without_internal_caller_is_an_entry(mini_ll):
    assert facts(entries_by_name(mini_ll)["demo_setup"], "D")


def test_internal_helper_only_called_directly_is_not_an_entry(mini_ll):
    assert "shared_helper" not in entries_by_name(mini_ll)


def test_entry_records_other_entries_it_is_reachable_from(mini_ll):
    es = entries_by_name(mini_ll)
    assert es["demo_release"].reachable_from == ["demo_close_all"]
    assert es["demo_setup"].reachable_from == []


def test_entry_carries_its_definition_site(mini_ll):
    e = entries_by_name(mini_ll)["demo_setup"]
    assert e.function.loc.line == line_of("void demo_setup(")
