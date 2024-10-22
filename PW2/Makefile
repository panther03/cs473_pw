all: report/report.pdf PW2.zip
.PHONY: PW2.zip

report/report.pdf: report/report.md
	cd report && pandoc report.md -o report.pdf --pdf-engine=xelatex

PW2.zip:
	cd ../ && zip -r PW2/PW2.zip PW2/programs/fractal_myflpt PW2/programs/fractal_fxpt