import numpy as np
import matplotlib.pyplot as plt
import argparse

def initialize_y_values(resolution):
    return np.zeros(resolution)

def adjust_intensity(intensity, min_intensity=1, max_intensity=5):
    return max(min_intensity, min(max_intensity, intensity))

def compute_linear_curve(x, curve_type, intensity):
    if curve_type == 'ease_in':
        return x ** intensity
    elif curve_type == 'ease_out':
        return (1 - x) ** intensity
    elif curve_type == 'ease_in_out':
        return (x * (1-x)) ** intensity
    else:
        raise ValueError(f"Invalid curve type '{curve_type}' for linear curve shape.")

def compute_cubic_curve(x, curve_type, intensity, base_intensity=3):
    if curve_type == 'ease_in':
        return x ** (base_intensity * intensity)
    elif curve_type == 'ease_out':
        return (1 - x) ** (base_intensity * intensity)
    elif curve_type == 'ease_in_out':
        return (x**3 * (1-x)**3) ** intensity
    else:
        raise ValueError(f"Invalid curve type '{curve_type}' for cubic curve shape.")

def generate_curve_values(start_value, end_value, resolution, curve_type, curve_shape, intensity):
    x = np.linspace(0, 1, resolution)
    intensity = adjust_intensity(intensity)
    
    if curve_shape == 'linear':
        y = compute_linear_curve(x, curve_type, intensity)
    elif curve_shape == 'cubic':
        y = compute_cubic_curve(x, curve_type, intensity)
    else:
        raise ValueError(f"Invalid curve shape '{curve_shape}'!")
    
    return y * (end_value - start_value) + start_value

def plot_curve(delayValues, values, curve_type, curve_shape, intensity):
    plt.plot(delayValues, '-o', label='Delay Values')
    plt.plot(values, '-o', label='Values')
    title = 'Distribution Along a {} {} Curve with Intensity {}'.format(curve_type.capitalize(), curve_shape.capitalize(), intensity)
    plt.title(title)
    plt.xlabel('Time')
    plt.ylabel('Value')
    plt.grid(True)
    plt.legend()  # Display the legend
    plt.show()

def save_to_gcode_file(filename, delayValues, values):
    args = parse_arguments()
    dvs = ["D" + str(round(dv)) for dv in delayValues]
    vs = [args.prefix + str(round(v)) for v in values]
    with open(filename, 'w') as file:
        for dv, v in zip(dvs, vs):
            file.write(f"{dv}\n{v}\n")

def generate_filename(args):
    filename = f"{args.curve_type}_{args.curve_shape}_Intensity{args.intensity}_Begin{args.start_value}_End{args.end_value}.gcode"
    # Replacing forward and backward slashes with underscores for compatibility
    filename = filename.replace('/', '_').replace('\\', '_')
    return filename

def generate_delay_values():
    args = parse_arguments()
    delayValues = generate_curve_values(0, args.duration, args.resolution, args.curve_type, args.curve_shape, args.intensity)

    return [round(num * args.duration / sum(delayValues)) for num in delayValues]

def main():
    args = parse_arguments()
    delayValues = generate_delay_values()
    x = np.linspace(args.start_value, args.end_value, args.resolution)
    values = compute_linear_curve(x, "ease_in", 1)

    filename = "maneuvers/" + generate_filename(args)
    save_to_gcode_file(filename, delayValues, values)
    print(f"Values saved to {filename}")

    plot_curve(delayValues, values, args.curve_type, args.curve_shape, args.intensity)

def parse_arguments():
    parser = argparse.ArgumentParser(description="Generate a distribution of values along a specified curve.")
    parser.add_argument('--start_value', type=float, default=0, help='Start value. Default is 0.')
    parser.add_argument('--end_value', type=float, default=100, help='End value. Default is 100.')
    parser.add_argument('--duration', type=float, default=1000, help='Duration of timeline in miliseconds. Default is 1000.') 
    parser.add_argument('--resolution', type=int, default=32, help='Number of points on the curve. Default is 100.')
    parser.add_argument('--curve_type', type=str, choices=['ease_in', 'ease_out', 'ease_in_out'], default='ease_in', help='Type of curve. Choices are [ease_in, ease_out, ease_in_out]. Default is ease_in.')
    parser.add_argument('--curve_shape', type=str, choices=['linear', 'cubic'], default='linear', help='Shape of curve. Choices are [linear, cubic]. Default is linear.')
    parser.add_argument('--intensity', type=float, default=2, help='Intensity of the curve. Default is 5. Less is probably more here (ie .1)')
    parser.add_argument('--prefix', type=str, help='Prefix before value is printed in gcode.')

    return parser.parse_args()

if __name__ == "__main__":
    main()
