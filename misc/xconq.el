(require 'hilit19)

;; define mode for xconq 5 periods

(defun period-mode ()
  "Major mode for editing xconq 5 periods"
  (interactive)
  (lisp-mode)
  (setq major-mode 'period-mode)
  (setq mode-name "xconq")
)

;; patterns for xconq 5 periods

(hilit-set-mode-patterns
 'period-mode
 '(
   (";.*" nil comment)
   ("#|" "|#" comment)
   ("\"[^\"]*\"" nil string)
   ("\\[[^][]*\\]" nil type)
   )
)

;; define mode for xconq 7 games

(defun gdl-mode ()
  "Major mode for editing xconq 7 games"
  (interactive)
  (lisp-mode)
  (setq major-mode 'gdl-mode)
  (setq mode-name "GDL")
)

;; patterns for xconq 7 games

(hilit-set-mode-patterns
 'gdl-mode
 '(
   (";.*" nil comment)
   ("#|" "|#" comment)
   (hilit-string-find ?\\ string)
   ("\\(table\\|define\\|undefine\\|set\\|add\\|quote\\|append\\|remove\\|list\\)[ \t\n]" 1 decl)
   ("\\(game-module\\|base-module\\)[ \t\n]" 1 define)
   ("\\(include\\)[ \t\n]" 1 include)
   ("[^a-z]\\(true\\|false\\|cond\\|if\\|else\\|end-if\\|not\\|or\\|and\\|do\\)[ \t\n]" 1 keyword)
   ))

(setq auto-mode-alist (append auto-mode-alist
			      (list '("\\.per$" . period-mode))
			      (list '("\\.g$"   . gdl-mode))

))

;;; And for editing the xconq C source code itself (GNU emacs 20, not
;;; sure about others):
(setq xconq-c-style
  '((c-basic-offset . 4)
    ))

(add-hook 'c-mode-common-hook
  (function (lambda ()
    (c-add-style "xconq" xconq-c-style t))))
