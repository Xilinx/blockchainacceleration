phys_opt_design -directive Explore
report_timing_summary -max_paths 10 -file timing_route_popt1.rpt
report_route_status -ignore_cache 
route_design -directive Explore
phys_opt_design -directive Explore
report_timing_summary -max_paths 10 -file timing_route_popt2.rpt
report_route_status -ignore_cache 
route_design -directive Explore
phys_opt_design -directive Explore
report_timing_summary -max_paths 10 -file timing_route_popt3.rpt
report_route_status -ignore_cache 
route_design -directive Explore
phys_opt_design -directive Explore
report_timing_summary -max_paths 10 -file timing_route_popt4.rpt
report_route_status -ignore_cache

