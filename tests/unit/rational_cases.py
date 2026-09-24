# Shared behavior contracts exercised through both VMs and the public CLI.
RATIONAL_FAILURES = (
    ('(1/3).nat()', 'exact Nat'),
    ('((-3)/1).nat()', 'exact Nat'),
    ('(1/3).dec()', 'non-terminating'),
    ('1/z', 'division by zero'),
    ('(1/3)/(z/1)', 'division by zero'),
    ('quotient(1, z)', 'division by zero'),
)


def rational_failure_source(expression):
    return 'main(): Void { z = nat_from_str("0"); print(' + expression + ') }'
