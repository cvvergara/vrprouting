-- creative commons

-- activate python
CREATE OR REPLACE PROCEDURE activate_python_venv(venv text)
LANGUAGE plpython3u AS
$BODY$
    import os
    import sys

    if sys.platform in ('win32', 'win64', 'cygwin'):
        activate_this = os.path.join(venv, 'Scripts', 'activate_this.py')
    else:
        activate_this = os.path.join(venv, 'bin', 'activate_this.py')

    exec(open(activate_this).read(), dict(__file__=activate_this))
$BODY$;

