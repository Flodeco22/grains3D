import sys

def print_bar(iteration, total, current_operation="", length=50):        
    percent = f"{100 * (iteration / float(total)):.1f}"
    filled_length = int(length * iteration // total)
    bar = '█' * filled_length + '-' * (length - filled_length)
    # Print the progress bar
    sys.stdout.write(f'\r|{bar}| {percent}% Complete - (')
    sys.stdout.write(current_operation)
    sys.stdout.write(')')
    sys.stdout.flush()