;;; Directory-traversal containment (security: MODERNIZATION-PLAN 6.4).
;;;
;;; An (include ...) names another module that the loader resolves against
;;; the library search path. Since the name is untrusted GDL content, a name
;;; that tries to escape the library dir -- an absolute path, a "..", or a
;;; backslashed Windows path -- must be REJECTED with a warning and NOT
;;; followed (see valid_untrusted_filename in kernel/util.cc and its use in
;;; include_module in kernel/read.cc). None of these files is ever opened.
;;;
;;; Like the other error*.g modules this is run with run_expect_diag: it must
;;; produce a diagnostic (the containment warnings) and, above all, must not
;;; crash or hang on the hostile input.

(game-module "error-traversal"
  (title "directory-traversal containment test"))

(include "../outside")
(include "../../etc/passwd")
(include "/etc/passwd")
(include "..\\..\\windows\\system32")
(include "foo/../../bar")
