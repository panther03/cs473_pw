all: report/report.pdf

report/report.pdf: report/report.md
	cd report && pandoc report.md -o report.pdf --pdf-engine=xelatex

#PW4.zip:
#	cd ../ && zip -r PW2/PW2.zip PW2/programs/fractal_myflpt PW2/programs/fractal_fxpt